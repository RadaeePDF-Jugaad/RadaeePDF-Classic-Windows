#include "pch.h"
#include "PDFCore.h"
#include "DXBlock.h"
#include "DXCommon.h"
#include "DXDevice.h"
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
int DXBlock::m_cell_size = 800;

DXBlock::~DXBlock()
{
	if (m_bmp) m_bmp->Release();
	m_bmp = NULL;
	m_dib = nullptr;
	m_page = nullptr;
	m_doc = nullptr;
}

void DXBlock::bk_render()
{
	if (m_status != 1) return;

	m_page = m_doc->GetPage(m_pageno);
	PDFDIB ^dib = ref new PDFDIB(m_w, m_h);
	m_page->RenderPrepare(dib);
	if (m_status != 1) return;

	int sw = (int)(m_doc->GetPageWidth(m_pageno) * m_scale);
	int sh = (int)(m_doc->GetPageHeight(m_pageno) * m_scale);
	if (sw <= m_w && sh <= m_h)
	{
		PDFMatrix ^mat = ref new PDFMatrix(m_scale, -m_scale, (m_w - sw) >> 1, (m_h + sh) >> 1);
		m_page->Render(dib, mat, true, RDPDFLib::pdf::PDF_RENDER_MODE::mode_best);
	}
	else
	{
		PDFMatrix ^mat = ref new PDFMatrix(m_scale, -m_scale, -m_x, m_ph - m_y);
		m_page->Render(dib, mat, true, RDPDFLib::pdf::PDF_RENDER_MODE::mode_best);
	}
	if (m_status == 1)
	{
		m_dib = dib;
		m_status = 2;
	}
}
void DXBlock::bk_destroy()
{
	if (m_page) { m_page->Close(); m_page = nullptr; }
	m_dib = nullptr;
	m_status = 0;
}

bool DXBlock::dx_end()
{
	if (m_status == 0 || m_status == -1) return false;
	if (m_status == 1 && m_page) m_page->RenderCancel();
	m_status = -1;
	if (m_bmp)
	{
		m_bmp->Release();
		m_bmp = NULL;
	}
	return true;
}

bool DXBlock::dx_make_bmp(DXDevice *device)
{
	if (m_bmp) return true;
	PDFDIB ^dib = m_dib;
	if (!dib) return false;
	m_dib = nullptr;
	m_bmp = dib->genDXBmp(device->GetD2DDeviceContext());
	return true;
}

void DXBlock::dx_draw(DXDevice *device, int left, int top, int right, int bottom)
{
	if (m_bmp)
		device->DXDrawBmp(m_bmp, left, top, right, bottom);
	else
		device->DXDrawRect(-1, left, top, right, bottom);
}
