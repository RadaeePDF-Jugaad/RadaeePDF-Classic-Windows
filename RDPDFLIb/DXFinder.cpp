#include "pch.h"
#include "PDFCore.h"
#include "DXPage.h"
#include "DXFinder.h"
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI;

void DXFinder::find_start(PDFDoc ^doc, int page_start, String ^str, bool match_case, bool whole)
{
	m_str = str;
	m_case = match_case;
	m_whole = whole;
	m_doc = doc;
	m_page_no = page_start;
	if (m_page)
	{
		if (m_finder)
		{
			m_finder = nullptr;
		}
		m_page->Close();
		m_page = nullptr;
	}
	m_page_find_index = -1;
	m_page_find_cnt = 0;
}

int DXFinder::find_prepare(int dir)
{
	if (!m_str) return 0;
	if (!is_cancel) m_eve.wait();
	m_dir = dir;
	m_eve.reset();
	if (!m_page)
	{
		is_cancel = false;
		return -1;
	}
	is_cancel = true;
	if (dir < 0)
	{
		if (m_page_find_index >= 0) m_page_find_index--;
		if (m_page_find_index < 0)
		{
			if (m_page_no <= 0)
			{
				return 0;
			}
			else
			{
				is_cancel = false;
				return -1;
			}
		}
		else
			return 1;
	}
	else
	{
		if (m_page_find_index < m_page_find_cnt) m_page_find_index++;
		if (m_page_find_index >= m_page_find_cnt)
		{
			if (m_page_no >= m_doc->PageCount - 1)
			{
				return 0;
			}
			else
			{
				is_cancel = false;
				return -1;
			}
		}
		else
			return 1;
	}
}
int DXFinder::find()
{
	int ret = 0;
	int pcnt = m_doc->PageCount;
	if (m_dir < 0)
	{
		while ((!m_page || m_page_find_index < 0) && m_page_no >= 0 && !is_cancel)
		{
			if (!m_page)
			{
				if (m_page_no >= pcnt) m_page_no = pcnt - 1;
				m_page = m_doc->GetPage(m_page_no);
				m_page->ObjsStart();
				//if (mSkipBlank)
				//	m_finder = m_page->GetFinder(m_str, m_case, m_whole, mSkipBlank);
				//else
					m_finder = m_page->GetFinder(m_str, m_case, m_whole);
				if (!m_finder) m_page_find_cnt = 0;
				else m_page_find_cnt = m_finder->GetCount();
				m_page_find_index = m_page_find_cnt - 1;
			}
			if (m_page_find_index < 0)
			{
				if (m_finder)
				{
					m_finder = nullptr;
				}
				m_page->Close();
				m_page = nullptr;
				m_page_find_cnt = 0;
				m_page_no--;
			}
		}
		if (is_cancel || m_page_no < 0)
		{
			if (m_finder)
			{
				m_finder = nullptr;
			}
			if (m_page)
			{
				m_page->Close();
				m_page = nullptr;
			}
			ret = 0;//find error, notify UI process
		}
		else
			ret = 1;//find finished, notify UI process
	}
	else
	{
		while ((!m_page || m_page_find_index >= m_page_find_cnt) && m_page_no < pcnt && !is_cancel)
		{
			if (!m_page)
			{
				if (m_page_no < 0) m_page_no = 0;
				m_page = m_doc->GetPage(m_page_no);
				m_page->ObjsStart();
				//if (mSkipBlank)
				//	m_finder = m_page->GetFinder(m_str, m_case, m_whole, mSkipBlank);
				//else
					m_finder = m_page->GetFinder(m_str, m_case, m_whole);
				if (!m_finder) m_page_find_cnt = 0;
				else m_page_find_cnt = m_finder->GetCount();
				m_page_find_index = 0;
			}
			if (m_page_find_index >= m_page_find_cnt)
			{
				if (m_finder)
				{
					m_finder = nullptr;
				}
				m_page->Close();
				m_page = nullptr;
				m_page_find_cnt = 0;
				m_page_no++;
			}
		}
		if (is_cancel || m_page_no >= pcnt)
		{
			if (m_finder)
			{
				m_finder = nullptr;
			}
			if (m_page)
			{
				m_page->Close();
				m_page = nullptr;
			}
			ret = 0;////find error, notify UI process
		}
		else
			ret = 1;//find finished, notify UI process
	}
	m_eve.notify();
	return ret;
}
void DXFinder::find_get_pos(PDFRect &rect)//get current found's bound.
{
	rect.left = 0;
	rect.top = 0;
	rect.right = 0;
	rect.bottom = 0;
	if (m_finder)
	{
		int ichar = m_finder->GetFirstChar(m_page_find_index);
		if (ichar < 0) return;
		rect = m_page->ObjsGetCharRect(ichar);
	}
}
void DXFinder::find_end()
{
	if (!is_cancel)
	{
		is_cancel = true;
		m_eve.wait();
	}
	m_str = nullptr;
	if (m_page)
	{
		if (m_finder)
		{
			m_finder = nullptr;
		}
		m_page->Close();
		m_page = nullptr;
	}
}

void DXFinder::find_draw(Canvas ^canvas, DXPage *page, int index, int scrollx, int scrolly)
{
	Color fclr;
	fclr.A = 64;
	fclr.B = 255;
	fclr.G = 0;
	fclr.R = 0;
	int ichar = m_finder->GetFirstChar(index);
	int ichar_end = ichar + m_str->Length();
	PDFRect rect = m_page->ObjsGetCharRect(ichar);
	PDFRect rect_word = rect;
	PDFRect rect_draw;
	ichar++;
	while (ichar < ichar_end)
	{
		rect = m_page->ObjsGetCharRect(ichar);
		float gap = (rect.bottom - rect.top) / 2;
		if (rect_word.top == rect.top && rect_word.bottom == rect.bottom &&
			rect_word.right + gap > rect.left && rect_word.left - gap < rect.right)
		{
			if (rect_word.left > rect.left) rect_word.left = rect.left;
			if (rect_word.right < rect.right) rect_word.right = rect.right;
		}
		else
		{
			rect_draw.left = page->GetVX(rect_word.left) - scrollx;
			rect_draw.top = page->GetVY(rect_word.bottom) - scrolly;
			rect_draw.right = page->GetVX(rect_word.right) - scrollx;
			rect_draw.bottom = page->GetVY(rect_word.top) - scrolly;
			DXCanvas::fill_rect(canvas, fclr, rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
			rect_word = rect;
		}
		ichar++;
	}
	rect_draw.left = page->GetVX(rect_word.left) - scrollx;
	rect_draw.top = page->GetVY(rect_word.bottom) - scrolly;
	rect_draw.right = page->GetVX(rect_word.right) - scrollx;
	rect_draw.bottom = page->GetVY(rect_word.top) - scrolly;
	DXCanvas::fill_rect(canvas, fclr, rect_draw.left, rect_draw.top, rect_draw.right, rect_draw.bottom);
}

void DXFinder::find_draw(Canvas ^canvas, DXPage *page, int scrollx, int scrolly)//draw current found
{
	if (!is_cancel)
	{
		m_eve.wait();
		is_cancel = true;
	}
	if (!m_str) return;
	if (m_finder && m_page_find_index >= 0 && m_page_find_index < m_page_find_cnt)
	{
		for (int index = 0; index < m_page_find_cnt; index++)
		{
			if (index == m_page_find_index)
				find_draw(canvas, page, index, scrollx, scrolly);
			else
				find_draw(canvas, page, index, scrollx, scrolly);
		}
	}
}
