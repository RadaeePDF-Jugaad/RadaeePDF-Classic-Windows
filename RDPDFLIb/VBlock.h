#pragma once
#include "PDFCore.h"
#include "VUtil.h"
using namespace RDPDFLib::pdf;
namespace RDPDFLib
{
	namespace view
	{
		class VBlock
		{
		public:
			static int m_cell_size;
			VBlock(const VBlock* src)
			{
				m_doc = src->m_doc;
				m_pageno = src->m_pageno;
				m_scale = src->m_scale;
				m_x = src->m_x;
				m_y = src->m_y;
				m_w = src->m_w;
				m_h = src->m_h;
				m_ph = src->m_ph;
				m_page = nullptr;
				m_status = 0;
				m_bmp = nullptr;
				m_drawn = false;
			}
			VBlock(pdf::PDFDoc^ doc, int pageno, float scale, int x, int y, int w, int h, int ph)
			{
				m_doc = doc;
				m_pageno = pageno;
				m_scale = scale;
				m_x = x;
				m_y = y;
				m_w = w;
				m_h = h;
				m_ph = ph;
				m_page = nullptr;
				m_status = 0;
				m_bmp = nullptr;
				m_drawn = false;
			}
			~VBlock()
			{
				m_bmp = nullptr;
				m_page = nullptr;
				m_doc = nullptr;
			}
			void bk_render();
			void bk_destroy();
			inline bool ui_start()
			{
				if (m_status != 0) return false;
				m_status = 1;
				if (!m_bmp)
				{
					m_bmp = ref new PDFBmp(m_w, m_h);
					m_bmp->Reset(-1);
				}
				if (!m_img)
				{
					m_img = ref new Image();
					m_img->Width = m_w;
					m_img->Height = m_h;
					Thickness margin = m_img->Margin;
					margin.Left = m_x;
					margin.Top = m_y;
					m_img->Margin = margin;
					m_img->Source = m_bmp->Data;
					//m_img->SetValue(Canvas::LeftProperty, m_x);
					//m_img->SetValue(Canvas::TopProperty, m_y);
				}
				return true;
			}
			bool ui_end(IVPanel^ panel)
			{
				if (m_status == 0 || m_status == -1) return false;
				m_status = -1;
				if (m_status == 1 && m_page) m_page->RenderCancel();
				if (m_img)
				{
					panel->vpRemoveBlock(m_img);
					m_img = nullptr;
				}
				if (m_bmp) m_bmp = nullptr;
				return true;
			}
			void ui_draw(IVPanel^ panel)
			{
				if (m_drawn || !m_bmp) return;
				if (!m_drawn)
				{
					panel->vpShowBlock(m_img);
					m_drawn = true;
				}
			}
			inline bool isCross(int left, int top, int right, int bottom)
			{
				return !(left >= m_x + m_w || right < m_x || top >= m_y + m_h || bottom < m_y);
			}
			inline int GetX()
			{
				return m_x;
			}
			inline int GetY()
			{
				return m_y;
			}
			inline int GetRight()
			{
				return m_x + m_w;
			}
			inline int GetBottom()
			{
				return m_y + m_h;
			}
			inline int GetW()
			{
				return m_w;
			}
			inline int GetH()
			{
				return m_h;
			}
			inline int GetPageno()
			{
				return m_pageno;
			}
		private:
			int m_x;
			int m_y;
			int m_w;
			int m_h;
			int m_ph;
			int m_status;
			float m_scale;
			PDFBmp^ m_bmp;
			Image^ m_img;
			PDFDoc^ m_doc;
			int m_pageno;
			PDFPage^ m_page;
			bool m_drawn;
		};
	}
}
