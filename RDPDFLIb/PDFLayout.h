#pragma once
#include "PDFCore.h"
#include "VUtil.h"
using namespace RDPDFLib::pdf;
namespace RDPDFLib
{
	namespace view
	{
		class PDFLayout
		{
		public:
			PDFLayout()
			{
				m_doc = nullptr;
				m_panel = nullptr;
				m_layw = 0;
				m_layh = 0;
				m_vx = 0;
				m_vy = 0;
				m_vw = 0;
				m_vh = 0;
				m_page_gap = 4;
				m_page_cnt = 0;
				m_scale_min = 1;
				m_scale_max = 10;
				m_pageno1 = -1;
				m_pageno2 = -1;
			}
			virtual ~PDFLayout()
			{
				vClose();
			}
			void vOpen(PDFDoc^ doc, IVPanel^ panel, int page_gap)
			{
				m_doc = doc;
				m_panel = panel;
				m_page_gap = page_gap;
			}
			void vClose()
			{
				m_doc = nullptr;
				m_panel = nullptr;
			}
			virtual void vLayout(float scale, bool zoom) = 0;
			virtual int vGetPage(int vx, int vy) = 0;
			void vDraw()
			{
			}
			inline int vGetX()
			{
				return m_vx;
			}
			inline void vSetX(int x)
			{
				if (x > m_layw - m_vw) x = m_layw - m_vw;
				if (x < 0) x = 0;
				m_vx = x;
			}
			inline int vGetY()
			{
				return m_vy;
			}
			inline void vSetY(int y)
			{
				if (y > m_layh - m_vh) y = m_layh - m_vh;
				if (y < 0) y = 0;
				m_vy = y;
			}
			inline int vGetLayoutW()
			{
				return m_layw;
			}
			inline int vGetLayoutH()
			{
				return m_layh;
			}
			PDFDoc^ m_doc;
			IVPanel^ m_panel;
			int m_layw;
			int m_layh;
			int m_vx;
			int m_vy;
			int m_vw;
			int m_vh;
			int m_page_gap;
			int m_page_cnt;
			float m_scale_min;
			float m_scale_max;
			int m_pageno1;
			int m_pageno2;
		};
		class PDFLayoutVert : public PDFLayout
		{
		public:
			PDFLayoutVert(int align) : PDFLayout()
			{
				m_align = align;
			}
			virtual void vLayout(float scale, bool zoom)
			{
				if (m_vw <= 0 || m_vh <= 0) return;
			}
			virtual int vGetPage(int vx, int vy)
			{
				if (m_vw <= 0 || m_vh <= 0) return - 1;
				return -1;
			}
			int m_align;
		};
	}
}