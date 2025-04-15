#pragma once

namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFDIB;
		ref class PDFDoc;
		ref class PDFPage;
	}
	namespace view
	{
		class DXDevice;
		class DXBlock
		{
		public:
			static int m_cell_size;
			DXBlock(const DXBlock *src)
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
				m_dib = nullptr;
				m_status = 0;
				m_bmp = NULL;
			}
			DXBlock(pdf::PDFDoc ^doc, int pageno, float scale, int x, int y, int w, int h, int ph)
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
				m_dib = nullptr;
				m_status = 0;
				m_bmp = NULL;
			}
			~DXBlock();
			void bk_render();
			void bk_destroy();
			inline bool dx_start()
			{
				if (m_status != 0) return false;
				m_status = 1;
				return true;
			}
			bool dx_end();
			bool dx_make_bmp(DXDevice *device);
			void dx_draw(DXDevice *device, int left, int top, int right, int bottom);
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
			ID2D1Bitmap *m_bmp;
			pdf::PDFDIB ^m_dib;
			pdf::PDFDoc ^m_doc;
			int m_pageno;
			pdf::PDFPage ^m_page;
		};
	}
}
