#include "pch.h"
#include "DXSel.h"
#include "PDFCore.h"
#include "DXCommon.h"
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI;

void DXSel::Clear()
{
	if (m_page)
	{
		m_page->Close();
		m_page = nullptr;
	}
}

void DXSel::GetRect1(float scale, float page_height, int orgx, int orgy, PDFRect &ret)
{
	ret.left = 0;
	ret.top = 0;
	ret.right = 0;
	ret.bottom = 0;
	if (m_index1 < 0 || m_index2 < 0 || !m_ok) return;
	PDFRect rect;
	if (m_swiped)
		rect = m_page->ObjsGetCharRect(m_index2);
	else
		rect = m_page->ObjsGetCharRect(m_index1);
	ret.left = (int)(rect.left * scale) + orgx;
	ret.top = (int)((page_height - rect.bottom) * scale) + orgy;
	ret.right = (int)(rect.right * scale) + orgx;
	ret.bottom = (int)((page_height - rect.top) * scale) + orgy;
}
void DXSel::GetRect2(float scale, float page_height, int orgx, int orgy, PDFRect &ret)
{
	ret.left = 0;
	ret.top = 0;
	ret.right = 0;
	ret.bottom = 0;
	if (m_index1 < 0 || m_index2 < 0 || !m_ok) return;
	PDFRect rect;
	if (m_swiped)
		rect = m_page->ObjsGetCharRect(m_index1);
	else
		rect = m_page->ObjsGetCharRect(m_index2);
	ret.left = (int)(rect.left * scale) + orgx;
	ret.top = (int)((page_height - rect.bottom) * scale) + orgy;
	ret.right = (int)(rect.right * scale) + orgx;
	ret.bottom = (int)((page_height - rect.top) * scale) + orgy;
}

void DXSel::SetSel(float x1, float y1, float x2, float y2)
{
	if (!m_ok)
	{
		m_page->ObjsStart();
		m_ok = true;
	}
	m_index1 = m_page->ObjsGetCharIndex(x1, y1);
	m_index2 = m_page->ObjsGetCharIndex(x2, y2);
	if (m_index1 > m_index2)
	{
		int tmp = m_index1;
		m_index1 = m_index2;
		m_index2 = tmp;
		m_swiped = true;
	}
	else
		m_swiped = false;
	m_index1 = m_page->ObjsAlignWord(m_index1, -1);
	m_index2 = m_page->ObjsAlignWord(m_index2, 1);
}

bool DXSel::SetSelMarkup(int type)
{
	if (m_index1 < 0 || m_index2 < 0 || !m_ok) return false;
	int color = 0xFFFFFF00;
	if (type == 1) color = 0xFF0000C0;
	if (type == 2) color = 0xFFC00000;
	if (type == 4) color = 0xFF00C000;
	return m_page->AddAnnotMarkup(m_index1, m_index2, color, type);
}

String ^DXSel::GetSelString()
{
	if (m_index1 < 0 || m_index2 < 0 || !m_ok) return nullptr;
	return m_page->ObjsGetString(m_index1, m_index2 + 1);
}

void DXSel::DrawSel(Canvas ^canvas, float scale, float page_height, int orgx, int orgy)
{
	if (m_index1 < 0 || m_index2 < 0 || !m_ok) return;
	PDFRect rect;
	PDFRect rect_word;
	PDFRect rect_draw;
	Color fclr;
	fclr.A = 64;
	fclr.B = 255;
	fclr.G = 0;
	fclr.R = 0;
	rect = m_page->ObjsGetCharRect(m_index1);
	rect_word = rect;
	int tmp = m_index1 + 1;
	while (tmp <= m_index2)
	{
		rect = m_page->ObjsGetCharRect(tmp);
		float gap = (rect.bottom - rect.top) * 0.5f;
		if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
			rect_word.right + gap > rect.left && rect_word.left - gap < rect.right)
		{
			if (rect_word.left > rect.left) rect_word.left = rect.left;
			if (rect_word.right < rect.right) rect_word.right = rect.right;
		}
		else
		{
			rect_draw.left = rect_word.left * scale;
			rect_draw.top = (page_height - rect_word.bottom) * scale;
			rect_draw.right = rect_word.right * scale;
			rect_draw.bottom = (page_height - rect_word.top) * scale;
			DXCanvas::fill_rect(canvas, fclr, (orgx + rect_draw.left), (orgy + rect_draw.top), (orgx + rect_draw.right), (orgy + rect_draw.bottom));
			rect_word = rect;
		}
		tmp++;
	}
	rect_draw.left = rect_word.left * scale;
	rect_draw.top = (page_height - rect_word.bottom) * scale;
	rect_draw.right = rect_word.right * scale;
	rect_draw.bottom = (page_height - rect_word.top) * scale;
	DXCanvas::fill_rect(canvas, fclr, (orgx + rect_draw.left), (orgy + rect_draw.top), (orgx + rect_draw.right), (orgy + rect_draw.bottom));
}
