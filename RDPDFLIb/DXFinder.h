#pragma once
#include "DXCommon.h"
namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFDoc;
		ref class PDFPage;
		ref class PDFFinder;
		value struct PDFRect;
	}
	namespace view
	{
		class DXPage;
		class DXFinder
		{
		public:
			DXFinder()
			{
				m_str = nullptr;
				m_case = false;
				m_whole = false;
				mSkipBlank = false;
				m_page_no = -1;
				m_page_find_index = -1;
				m_page_find_cnt = 0;
				m_page = nullptr;
				m_doc = nullptr;
				m_finder = nullptr;
				m_dir = 0;
				is_cancel = true;
			}
			~DXFinder()
			{
				m_str = nullptr;
				m_finder = nullptr;
				m_page = nullptr;
				m_doc = nullptr;
			}
			void find_start(pdf::PDFDoc ^doc, int page_start, Platform::String ^str, bool match_case, bool whole);
			int find_prepare(int dir);
			int find();
			void find_get_pos(pdf::PDFRect &rect);//get current found's bound.
			inline int find_get_page()//get current found's page NO
			{
				return m_page_no;
			}
			void find_end();
			void find_draw(Windows::UI::Xaml::Controls::Canvas ^canvas, DXPage *page, int scrollx, int scrolly);//draw current found
		private:
			void find_draw(Windows::UI::Xaml::Controls::Canvas ^canvas, DXPage *page, int index, int scrollx, int scrolly);
			Platform::String ^m_str;
			bool m_case;
			bool m_whole;
			bool mSkipBlank;
			int m_page_no;
			int m_page_find_index;
			int m_page_find_cnt;
			pdf::PDFPage ^m_page;
			pdf::PDFDoc ^m_doc;
			pdf::PDFFinder ^m_finder;
			int m_dir;
			bool is_cancel;
			DXEvent m_eve;
		};
	}
}