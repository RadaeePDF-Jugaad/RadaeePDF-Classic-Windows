#include "pch.h"
#include "DXBlock.h"
#include "DXPage.h"
#include "DXThread.h"
#include "PDFCore.h"
using namespace RDPDFLib::view;
using namespace RDPDFLib::pdf;

bool DXPage::dx_layout(int x, int y, float scale)
{
	m_left = x;
	m_top = y;
	m_pw = (int)(m_doc->GetPageWidth(m_pageno) * scale);
	m_ph = (int)(m_doc->GetPageHeight(m_pageno) * scale);
	m_right = m_left + m_pw;
	m_bottom = m_top + m_ph;
	if (m_scale == scale) return false;
	m_scale = scale;
	return true;
}
/*
void DXPage::dx_layout(int w, int h)
{
	m_left = 0;
	m_top = 0;
	m_right = m_left + w;
	m_bottom = m_top + h;
	float pw = m_doc->GetPageWidth(m_pageno);
	float ph = m_doc->GetPageHeight(m_pageno);
	float scale = w / pw;
	m_scale = h / ph;
	if (m_scale > scale) m_scale = scale;
	m_pw = (int)(pw * m_scale);
	m_ph = (int)(ph * m_scale);
}
*/
void DXPage::dx_alloc()
{
	int width = m_pw;
	int height = m_ph;
	int csize2 = DXBlock::m_cell_size << 1;
	if (DXBlock::m_cell_size > 0 && (width >= csize2 || height >= csize2))
	{
		int xcnt = (width + DXBlock::m_cell_size - 1) / DXBlock::m_cell_size;
		int ycnt = (height + DXBlock::m_cell_size - 1) / DXBlock::m_cell_size;
		m_blks_cnt = xcnt * ycnt;
		m_blks = (DXBlock **)malloc(sizeof(DXBlock *)  * m_blks_cnt);
		int cury = 0;
		for (int yb = 0; yb < ycnt; yb++)
		{
			int curx = 0;
			int bheight = (yb < ycnt - 1) ? DXBlock::m_cell_size : height - cury;
			for (int xb = 0; xb < xcnt; xb++)
			{
				int bwidth = (xb < xcnt - 1) ? DXBlock::m_cell_size : width - curx;
				m_blks[yb * xcnt + xb] = new DXBlock(m_doc, m_pageno, m_scale, curx, cury, bwidth, bheight, height);
				curx += bwidth;
			}
			cury += bheight;
		}
	}
	else
	{
		m_blks = (DXBlock **)malloc(sizeof(DXBlock *));
		m_blks[0] = new DXBlock(m_doc, m_pageno, m_scale, 0, 0, m_pw, m_ph, height);
		m_blks_cnt = 1;
	}
}

void DXPage::dx_render(DXThread &thread)
{
	thread.render_start(m_blks[0]);
}

void DXPage::dx_draw(DXThread &thread, DXDevice *device, int orgx, int orgy, int w, int h)
{
	if (m_dirty)
	{
		m_dirty = false;
		dx_zoom_start(thread);
		dx_alloc();
	}
	int left = m_left - orgx;
	int top = m_top - orgy;
	//int right = m_right - orgx;
	//int bottom = m_bottom - orgy;
	if (m_blks)
	{
		bool all_ok = true;
		int cleft = -left - DXBlock::m_cell_size;
		int ctop = -top - DXBlock::m_cell_size;
		int cright = w - left + DXBlock::m_cell_size;
		int cbottom = h - top + DXBlock::m_cell_size;
		for (int ib = 0; ib < m_blks_cnt; ib++)
		{
			DXBlock *glb = m_blks[ib];
			if (glb->isCross(cleft, ctop, cright, cbottom))
			{
				if (!glb->dx_make_bmp(device))
				{
					all_ok = false;
					thread.render_start(glb);
				}
				//else do noting.
			}
			else if (thread.render_end(glb)) m_blks[ib] = new DXBlock(glb);
		}
		if (all_ok) dx_end_zoom(thread);//destroy zoom cache and draw blk.
		if (!m_blks_zoom)
		{
			for (int ib = 0; ib < m_blks_cnt; ib++)
			{
				DXBlock *glb = m_blks[ib];
				cleft = left + glb->GetX();
				ctop = top + glb->GetY();
				cright = left + glb->GetRight();
				cbottom = top + glb->GetBottom();
				if (cright <= 0 || cleft >= w || cbottom <= 0 || ctop >= h) continue;
				glb->dx_draw(device, cleft, ctop, cright, cbottom);
			}
		}
	}
	if (!m_blks_zoom) return;
	DXBlock *glb = m_blks_zoom[m_blks_zoom_cnt - 1];
	int srcw = glb->GetRight();
	int srch = glb->GetBottom();
	int dstw = m_pw;
	int dsth = m_ph;
	for (int ibb = 0; ibb < m_blks_zoom_cnt; ibb++)
	{
		glb = m_blks_zoom[ibb];
		int zleft = left + glb->GetX() * dstw / srcw;
		int ztop = top + glb->GetY() * dsth / srch;
		int zright = left + glb->GetRight() * dstw / srcw;
		int zbottom = top + glb->GetBottom() * dsth / srch;
		if (zright <= 0 || zleft >= w || zbottom <= 0 || ztop >= h) continue;
		glb->dx_draw(device, zleft, ztop, zright, zbottom);
	}
}

void DXPage::dx_end(DXThread &thread)
{
	if (!m_blks) return;
	for (int cur = 0; cur < m_blks_cnt; cur++)
	{
		DXBlock *glb = m_blks[cur];
		if (thread.render_end(glb)) m_blks[cur] = new DXBlock(glb);
	}
}

void DXPage::dx_end_zoom(DXThread &thread)
{
	if (!m_blks_zoom) return;
	for (int cur = 0; cur < m_blks_zoom_cnt; cur++)
		thread.render_end(m_blks_zoom[cur]);
	free(m_blks_zoom);
	m_blks_zoom = NULL;
	m_blks_zoom_cnt = 0;
}

void DXPage::dx_zoom_start(DXThread &thread)
{
	if (m_blks_zoom)
	{
		if (m_blks)
		{
			for (int cur = 0; cur < m_blks_cnt; cur++)
			{
				DXBlock *glb = m_blks[cur];
				thread.render_end(glb);
			}
			free(m_blks);
			m_blks = NULL;
		}
		return;
	}
	m_blks_zoom = m_blks;
	m_blks_zoom_cnt = m_blks_cnt;
	m_blks = NULL;
	m_blks_cnt = 0;
	return;
}

PDFMatrix ^DXPage::CreateInvertMatrix(float scrollx, float scrolly)
{
	PDFMatrix ^mat = ref new PDFMatrix(m_scale, -m_scale, m_left - scrollx, m_bottom - scrolly);
	mat->Invert();
	return mat;
}
