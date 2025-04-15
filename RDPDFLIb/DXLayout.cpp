#include "pch.h"
#include "PDFCore.h"
#include "DXLayout.h"
#include "DXBlock.h"
#include "DXDevice.h"
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;

void DXLayout::vOpen(PDFDoc ^doc, DXDevice *device, int page_gap)
{
	m_doc = doc;
	m_device = device;
	m_page_gap = page_gap;
	m_thread.start();
	m_page_cnt = m_doc->PageCount;
	m_pages = (DXPage *)calloc(sizeof(DXPage), m_page_cnt);
	DXPage *cur = m_pages;
	for (int pcur = 0; pcur < m_page_cnt; pcur++)
	{
		cur->dx_init(doc, pcur);
		cur++;
	}
}

void DXLayout::vClose()
{
	if (m_thread.is_run())
	{
		DXPage *cur = m_pages;
		DXPage *end = cur + m_page_cnt;
		while(cur < end)
		{
			cur->dx_end(m_thread);
			cur->dx_end_zoom(m_thread);
		}
		m_thread.destroy();
		free(m_pages);
	}
	m_doc = nullptr;
	m_pages = NULL;
	m_scale = -1;
	m_layw = 0;
	m_layh = 0;
	m_vw = 0;
	m_vh = 0;
}

void DXLayout::dx_flush_range()
{
	if (!m_scroller.update() && m_pageno2 > m_pageno1) return;
	int pageno1 = vGetPage(-DXBlock::m_cell_size, -DXBlock::m_cell_size);
	int pageno2 = vGetPage(m_vw + DXBlock::m_cell_size, m_vh + DXBlock::m_cell_size);
	if (pageno1 >= 0 && pageno2 >= 0)
	{
		if (pageno1 > pageno2)
		{
			int tmp = pageno1;
			pageno1 = pageno2;
			pageno2 = tmp;
		}
		pageno2++;
		if (m_pageno1 < pageno1)
		{
			int start = m_pageno1;
			int end = pageno1;
			if (end > m_pageno2) end = m_pageno2;
			DXPage *pcur = m_pages + start;
			DXPage *pend = m_pages + end;
			while (pcur < pend)
			{
				pcur->dx_end_zoom(m_thread);
				pcur->dx_end(m_thread);
				pcur++;
			}
		}
		if (m_pageno2 > pageno2)
		{
			int start = pageno2;
			int end = m_pageno2;
			if (start < m_pageno1) start = m_pageno1;
			DXPage *pcur = m_pages + start;
			DXPage *pend = m_pages + end;
			while (pcur < pend)
			{
				pcur->dx_end_zoom(m_thread);
				pcur->dx_end(m_thread);
				pcur++;
			}
		}
	}
	else
	{
		DXPage *pcur = m_pages + m_pageno1;
		DXPage *pend = m_pages + m_pageno2;
		while (pcur < pend)
		{
			pcur->dx_end_zoom(m_thread);
			pcur->dx_end(m_thread);
			pcur++;
		}
	}
	m_pageno1 = pageno1;
	m_pageno2 = pageno2;
	if (m_cur_pageno != m_pageno1 && m_listener)
	{
		m_cur_pageno = m_pageno1;
		m_listener->onVPageChanged(m_cur_pageno);
	}
}

void DXLayout::vDraw()
{
	if (!m_doc) return;
	m_locker.lock();
	dx_flush_range();
	m_device->lock();
	ID2D1DeviceContext *ctx = m_device->GetD2DDeviceContext();
	D2D1_COLOR_F clr;
	clr.a = 1;
	clr.b = 0.5;
	clr.g = 0.5;
	clr.r = 1;
	ctx->BeginDraw();
	ctx->Clear(clr);
	int vx = vGetX();
	int vy = vGetY();
	for (int pcur = m_pageno1; pcur < m_pageno2; pcur++)
		m_pages[pcur].dx_draw(m_thread, m_device, vx, vy, m_vw, m_vh);
	ctx->EndDraw();
	m_device->unlock();
	m_locker.unlock();
}

void DXLayout::dx_find_goto()
{
	if (!m_pages) return;
	int pg = m_finder.find_get_page();
	if (pg < 0 || pg >= m_doc->PageCount) return;
	int x = vGetX();
	int y = vGetY();
	PDFRect pos;
	m_finder.find_get_pos(pos);
	if (pos.left >= pos.right) return;
	pos.left = m_pages[pg].GetVX(pos.left);
	pos.top = m_pages[pg].GetVY(pos.top);
	pos.right = m_pages[pg].GetVX(pos.right);
	pos.bottom = m_pages[pg].GetVY(pos.bottom);
	if (x > pos.left - m_vw / 8) x = (int)pos.left - m_vw / 8;
	if (x < pos.right - m_vw * 7 / 8) x = (int)pos.right - m_vw * 7 / 8;
	if (y > pos.top - m_vh / 8) y = (int)pos.top - m_vh / 8;
	if (y < pos.bottom - m_vh * 7 / 8) y = (int)pos.bottom - m_vh * 7 / 8;
	if (x > m_layw - m_vw) x = m_layw - m_vw;
	if (x < 0) x = 0;
	if (y > m_layh - m_vh) y = m_layh - m_vh;
	if (y < 0) y = 0;
	vEndScroll();
	m_scroller.setFinalX(x);
	m_scroller.setFinalY(y);
}

void DXLayout::onVCacheRendered(DXBlock *blk)
{
	if (m_listener) m_listener->onVCacheRendered(blk->GetPageno());
}

void DXLayout::onVFound(int ret)
{
	if (ret == 1)
	{
		dx_find_goto();
		if (m_listener) m_listener->onVFound(true);
	}
	else if(m_listener) m_listener->onVFound(false);
}


void DXLayoutVert::vLayout(float scale, bool zoom)
{
	if (m_vw <= 0 || m_vh <= 0) return;
	m_locker.lock();
	float maxw = 0;
	PDFPoint size = m_doc->MaxPageSize;
	m_scale_min = (m_vw - m_page_gap) / size.x;
	float max_scale = m_scale_min * m_max_zoom;
	if (scale < m_scale_min) scale = m_scale_min;
	if (scale > max_scale) scale = max_scale;
	if (m_scale == scale)
	{
		m_locker.unlock();
		return;
	}
	m_scale = scale;
	m_layw = (int)(size.x * m_scale) + m_page_gap;
	int y = m_page_gap >> 1;
	for (int pcur = 0; pcur < m_page_cnt; pcur++)
	{
		int x;
		float pg_scale = m_scale;
		float pg_width = m_doc->GetPageWidth(pcur);
		if (m_same_width)
			pg_scale = m_scale * size.x / pg_width;
		switch (m_align)
		{
		case ALIGN_LEFT:
			x = m_page_gap >> 1;
			break;
		case ALIGN_RIGHT:
			x = m_layw - (m_page_gap >> 1);
			break;
		default:
			x = (m_layw - (int)(m_doc->GetPageWidth(pcur) * pg_scale)) >> 1;
			break;
		}
		if (m_pages[pcur].dx_layout(x, y, pg_scale) && !zoom) m_pages[pcur].dx_alloc();
		y += (int)(m_doc->GetPageHeight(pcur) * pg_scale) + m_page_gap;
	}
	m_layh = y;
	m_locker.unlock();
}
