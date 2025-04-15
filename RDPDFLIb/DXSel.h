#pragma once

namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFPage;
		value struct PDFRect;
	}
	namespace view
	{
		class DXSel
		{
		protected:
			pdf::PDFPage ^m_page;
			int m_index1;
			int m_index2;
			bool m_ok = false;
			bool m_swiped = false;

		public:
			DXSel(pdf::PDFPage ^page)
			{
				m_page = page;
				m_index1 = -1;
				m_index2 = -1;
				m_ok = false;
				m_swiped = false;
			}
			void Clear();
			void GetRect1(float scale, float page_height, int orgx, int orgy, pdf::PDFRect &ret);
			void GetRect2(float scale, float page_height, int orgx, int orgy, pdf::PDFRect &ret);
			void SetSel(float x1, float y1, float x2, float y2);
			inline pdf::PDFPage ^GetPage()
			{
				return m_page;
			}
			bool SetSelMarkup(int type);
			Platform::String ^GetSelString();
			void DrawSel(Windows::UI::Xaml::Controls::Canvas ^canvas, float scale, float page_height, int orgx, int orgy);
		};
	}
}