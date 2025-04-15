#pragma once
#include "DXThread.h"
#include "DXScroller.h"
#include "DXPage.h"
#include "DXFinder.h"
namespace RDPDFLib
{
	namespace pdf
	{
		ref class PDFDoc;
	}
	namespace view
	{
#define m_max_zoom 10
		class DXBlock;
		class DXDevice;
		public interface class IDXLayoutCallback
		{
			virtual void onVPageChanged(int pageno) = 0;
			virtual void onVFound(bool found) = 0;
			virtual void onVCacheRendered(int pageno) = 0;
		};
		struct PDFPos
		{
			float x;
			float y;
			int pageno;
		};
		class DXLayout : public IDXThreadCallback
		{
		public:
			DXLayout(Windows::UI::Xaml::Controls::Canvas ^canvas, IDXLayoutCallback ^callback)
				:m_thread(canvas->Dispatcher, this)
			{
				m_listener = callback;
				m_scale = -1;
				m_layw = 0;
				m_layh = 0;
				m_vw = 0;
				m_vh = 0;
				m_cur_pageno = -1;
			}
			void vOpen(pdf::PDFDoc ^doc, DXDevice *device, int page_gap);
			void vClose();
			void vResize(int cx, int cy)
			{
				if (cx <= 0 || cy <= 0) return;
				if (cx == m_vw && cy == m_vh) return;
				vEndScroll();
				PDFPos pos;
				vGetPos(m_vw >> 1, m_vh >> 1, pos);
				m_vw = cx;
				m_vh = cy;
				vLayout(m_scale, false);
				vSetPos(m_vw >> 1, m_vh >> 1, pos);
				vMoveEnd();
			}
			inline DXPage *vGetPage(int pageno)
			{
				return m_pages + pageno;
			}
			virtual int vGetPage(int vx, int vy) = 0;
			virtual void vLayout(float scale, bool zoom) = 0;
			void vDraw();
			inline int vGetX()
			{
				int x = m_scroller.getCurrX();
				if (x > m_layw - m_vw) x = m_layw - m_vw;
				if (x < 0) x = 0;
				return x;
			}
			inline void vSetX(int x)
			{
				if (x > m_layw - m_vw) x = m_layw - m_vw;
				if (x < 0) x = 0;
				m_scroller.setFinalX(x);
			}
			inline int vGetY()
			{
				int y = m_scroller.getCurrY();
				if (y > m_layh - m_vh) y = m_layh - m_vh;
				if (y < 0) y = 0;
				return y;
			}
			inline void vSetY(int y)
			{
				if (y > m_layh - m_vh) y = m_layh - m_vh;
				if (y < 0) y = 0;
				m_scroller.setFinalY(y);
			}
			void vSetPos(int vx, int vy, const PDFPos &pos)
			{
				if (pos.pageno < 0) return;
				DXPage *gpage = m_pages + pos.pageno;
				vSetX(gpage->GetVX(pos.x) - vx);
				vSetY(gpage->GetVY(pos.y) - vy);
				m_scroller.update();//update scroller value immediately.
				m_scroller.forceFinished(false);
			}
			void vGetPos(int vx, int vy, PDFPos &pos)
			{
				pos.pageno = -1;
				pos.x = 0;
				pos.y = 0;
				int pgno = vGetPage(vx, vy);
				if (pgno < 0 || pgno >= m_page_cnt) return;
				DXPage *gpage = m_pages + pgno;
				pos.pageno = pgno;
				pos.x = gpage->GetPDFX(vGetX() + vx);
				pos.y = gpage->GetPDFY(vGetY() + vy);
			}
			void vEndScroll()
			{
				if (!m_doc) return;
				if (m_scroller.isFinished()) return;
				m_scroller.update();
				m_scroller.setFinalX(m_scroller.getCurrX());
				m_scroller.setFinalY(m_scroller.getCurrY());
				m_scroller.abortAnimation();
				m_scroller.forceFinished(true);
			}
			virtual bool vFling(float vx, float vy)
			{
				m_scroller.update();
				m_scroller.forceFinished(true);
				m_scroller.fling(vGetX(), vGetY(), (int)-vx, (int)-vy, -m_vw, m_layw, -m_vh, m_layh);
				return true;
			}
			virtual bool vWheel(int delta)
			{
				vEndScroll();
				m_scroller.startScroll(vGetX(), vGetY(), 0, -delta, 50);
				return true;
			}
			virtual void vMoveEnd() {}
			inline float vGetZoom() { return m_scale / m_scale_min; }
			inline void vZoomSet(float zoom)
			{
				vLayout(zoom * m_scale_min, true);
			}
			inline void vZoomStart()
			{
				if (m_pageno1 < 0 || m_pageno2 < 0) return;
				vEndScroll();
				m_locker.lock();
				DXPage *cur = m_pages;
				DXPage *end = cur + m_page_cnt;
				while(cur < end)
				{
					cur->dx_zoom_start(m_thread);
					cur++;
				}
				m_locker.unlock();
			}
			inline void vZoomConfirm()
			{
				m_locker.lock();
				for (int cur = 0; cur < m_page_cnt; cur++)
				{
					if (cur < m_pageno1 || cur >= m_pageno2)
						m_pages[cur].dx_end_zoom(m_thread);
					m_pages[cur].dx_end(m_thread);
					m_pages[cur].dx_alloc();
				}
				m_locker.unlock();
			}
			/**
			 * render page again, after page modified.
			 * @param page page object obtained by vGetPage()
			 */
			inline void vRenderPage(DXPage *page)
			{
				if (!m_pages || !page) return;
				page->dx_set_dirty();
			}
			void vGotoPage(int pageno)
			{
				if (!m_pages || pageno < 0 || pageno >= m_page_cnt) return;
				float x = m_pages[pageno].GetLeft() - (m_page_gap >> 1);
				float y = m_pages[pageno].GetTop() - (m_page_gap >> 1);
				if (x > m_layw - m_vw) x = m_layw - m_vw;
				if (x < 0) x = 0;
				if (y > m_layh - m_vh) y = m_layh - m_vh;
				if (y < 0) y = 0;
				m_scroller.setFinalX((int)x);
				m_scroller.setFinalY((int)y);
			}
			inline void vFindStart(Platform::String ^key, bool match_case, bool whole_word)
			{
				if (!m_pages) return;
				int pageno = vGetPage(0, 0);
				m_finder.find_end();
				m_finder.find_start(m_doc, pageno, key, match_case, whole_word);
			}
			int vFind(int dir)
			{
				if (!m_pages) return -1;
				int ret = m_finder.find_prepare(dir);
				if (ret == 1)
				{
					if (m_listener)
						m_listener->onVFound(true);
					dx_find_goto();
					return 0;//succeeded
				}
				if (ret == 0)
				{
					if (m_listener)
						m_listener->onVFound(false);
					return -1;//failed
				}
				m_thread.find_start(&m_finder);//need thread operation.
				return 1;
			}
			void vFindEnd()
			{
				if (!m_pages) return;
				m_finder.find_end();
			}
			void vFindDraw()
			{
				int pageno0 = m_finder.find_get_page();
				if (pageno0 >= m_pageno1 && pageno0 < m_pageno2)
					m_finder.find_draw(m_canvas, m_pages + pageno0, vGetX(), vGetY());
			}
		protected:
			void dx_find_goto();
			DXPage *m_pages;
			int m_page_gap;
			int m_page_cnt;
			int m_cur_pageno;
			pdf::PDFDoc ^m_doc;
			DXFinder m_finder;
			int m_pageno1;
			int m_pageno2;
			float m_scale;
			float m_scale_min;
			float m_scale_max;
			int m_layw;
			int m_layh;
			int m_vw;
			int m_vh;
			IDXLayoutCallback ^m_listener;
			Windows::UI::Xaml::Controls::Canvas ^m_canvas;
			DXThread m_thread;
			DXScroller m_scroller;
			DXDevice *m_device;
			DXLocker m_locker;
			void dx_flush_range();
			void onVCacheRendered(DXBlock *blk);
			void onVFound(int ret);
		};

		class DXLayoutVert : public DXLayout
		{
		public:
			enum ALIGN_TYPE
			{
				ALIGN_CENTER = 0,
				ALIGN_LEFT = 1,
				ALIGN_RIGHT = 2,
			};
			DXLayoutVert(Windows::UI::Xaml::Controls::Canvas ^canvas, IDXLayoutCallback ^callback, ALIGN_TYPE align, bool same_width)
				:DXLayout(canvas, callback)
			{
				m_same_width = same_width;
				m_align = align;
			}
			virtual int vGetPage(int vx, int vy)
			{
				if (m_vw <= 0 || m_vh <= 0) return -1;
				vy += vGetY();
				int pl = 0;
				int pr = m_page_cnt - 1;
				int hg = (m_page_gap >> 1);
				while (pr >= pl)
				{
					int mid = (pl + pr) >> 1;
					DXPage *pmid = m_pages + mid;
					if (vy < pmid->GetTop() - hg)
						pr = mid - 1;
					else if (vy >= pmid->GetBottom() + hg)
						pl = mid + 1;
					else return mid;
				}
				return (pr < 0) ? 0 : pr;
			}
			virtual void vLayout(float scale, bool zoom);
		private:
			ALIGN_TYPE m_align;
			bool m_same_width;
		};

		class DXLayoutDual : public DXLayout
		{
		public:
			enum SCALE_MODE
			{
				SCALE_NONE = 0,
				SCALE_SAME_WIDTH = 1,
				SCALE_SAME_HEIGHT = 2,
				SCALE_FIT = 3,
			};
			enum ALIGN_TYPE
			{
				ALIGN_CENTER = 0,
				ALIGN_TOP = 1,
				ALIGN_BOTTOM = 2,
			};
			DXLayoutDual(Windows::UI::Xaml::Controls::Canvas ^canvas, IDXLayoutCallback ^callback, ALIGN_TYPE align, SCALE_MODE scale_mode, bool rtol, bool *horz_dual, int horz_dual_cnt, bool *vert_dual, int vert_dual_cnt)
				:DXLayout(canvas, callback)
			{
				m_horz_dual = horz_dual;
				m_horz_dual_cnt = horz_dual_cnt;
				m_vert_dual = vert_dual;
				m_vert_dual_cnt = vert_dual_cnt;
				m_align_type = align;
				m_scale_mode = scale_mode;
				m_rtol = rtol;
				m_cells = NULL;
				m_cells_cnt = 0;
			}
			int vGetPage(int vx, int vy)
			{
				if (m_vw <= 0 || m_vh <= 0) return -1;
				vx += vGetX();
				int pl = 0;
				int pr = m_cells_cnt - 1;
				int hg = (m_page_gap >> 1);
				while (pr >= pl) {
					int mid = (pl + pr) >> 1;
					PDFCell *pmid = m_cells + mid;
					if (vx < pmid->left - hg)
						pr = mid - 1;
					else if (vx >= pmid->right + hg)
						pl = mid + 1;
					else {
						DXPage *page = m_pages + pmid->page_left;
						if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
						else return pmid->page_left;
					}
				}
				int mid = (pr < 0) ? 0 : pr;
				PDFCell *pmid = m_cells + mid;
				DXPage *page = m_pages + pmid->page_left;
				if (vx >= page->GetRight() && pmid->page_right >= 0) return pmid->page_right;
				else return pmid->page_left;
			}
			virtual bool vWheel(int delta)
			{
				vEndScroll();
				vFling(delta * 32, 0);
				return true;
			}
		private:
			bool *m_vert_dual;
			int m_vert_dual_cnt;
			bool *m_horz_dual;
			int m_horz_dual_cnt;
			bool m_rtol;
			ALIGN_TYPE m_align_type;
			SCALE_MODE m_scale_mode;
			struct PDFCell
			{
				int left;
				int right;
				float scale;
				int page_left;
				int page_right;
			};
			PDFCell *m_cells;
			int m_cells_cnt;
			inline void do_scroll(int x, int y, int dx, int dy)
			{
				float secx = dx * 512 / m_vw;
				float secy = dy * 512 / m_vh;
				int sec = (int)sqrtf(secx * secx + secy * secy);
				m_scroller.startScroll(x, y, dx, dy, sec);
			}
			int get_cell(int vx)
			{
				if (!m_pages || m_page_cnt <= 0 || !m_cells) return -1;
				int left = 0;
				int right = m_cells_cnt - 1;
				while (left <= right)
				{
					int mid = (left + right) >> 1;
					PDFCell *pg1 = m_cells + mid;
					if (vx < pg1->left)
					{
						right = mid - 1;
					}
					else if (vx > pg1->right)
					{
						left = mid + 1;
					}
					else
					{
						return mid;
					}
				}
				if (right < 0) return -1;
				else return m_cells_cnt;
			}
			static inline bool dual_at(bool *para, int para_cnt, int icell)
			{
				if (!para || icell >= para_cnt) return false;
				return para[icell];
			}
			void layout_ltor(float scale, bool zoom, bool *para, int para_cnt)
			{
				if (m_vw <= 0 || m_vh <= 0) return;
				float maxw = 0;
				float maxh = 0;
				int minscalew = 0x40000000;
				int minscaleh = 0x40000000;
				int pcur = 0;
				int ccnt = 0;
				while (pcur < m_page_cnt)
				{
					float cw = m_doc->GetPageWidth(pcur);
					float ch = m_doc->GetPageHeight(pcur);
					if (dual_at(para, para_cnt, ccnt))
					{
						if (pcur < m_page_cnt - 1)
						{
							cw += m_doc->GetPageWidth(pcur + 1);
							float ch2 = m_doc->GetPageHeight(pcur + 1);
							if (ch < ch2) ch = ch2;
							pcur += 2;
						}
						else pcur++;
					}
					else pcur++;
					if (maxw < cw) maxw = cw;
					if (maxh < ch) maxh = ch;
					float scalew = (m_vw - m_page_gap) / cw;
					float scaleh = (m_vh - m_page_gap) / ch;
					if (scalew > scaleh) scalew = scaleh;
					cw *= scalew;
					ch *= scalew;
					if (minscalew > (int)cw) minscalew = (int)cw;
					if (minscaleh > (int)ch) minscaleh = (int)ch;
					ccnt++;
				}

				bool changed = (!m_cells || m_cells_cnt != ccnt);
				if (changed)
				{
					if (m_cells) free(m_cells);
					m_cells = (PDFCell *)calloc(ccnt, sizeof(PDFCell));
					m_cells_cnt = ccnt;
				}
				m_scale_min = (float)(m_vw - m_page_gap) / maxw;
				float scalew;
				float scaleh = (float)(m_vh - m_page_gap) / maxh;
				if (m_scale_min > scaleh) m_scale_min = scaleh;
				float max_scale = m_scale_min * m_max_zoom;
				if (scale < m_scale_min) scale = m_scale_min;
				if (scale > max_scale) scale = max_scale;
				//if(m_scale == scale) return;
				m_scale = scale;
				m_layw = 0;
				m_layh = 0;
				pcur = 0;
				for (int ccur = 0; ccur < ccnt; ccur++)
				{
					float cw = m_doc->GetPageWidth(pcur);
					float ch = m_doc->GetPageHeight(pcur);
					PDFCell *cell = m_cells + ccur;
					if (dual_at(para, para_cnt, ccur))
					{
						if (pcur < m_page_cnt - 1)
						{
							cw += m_doc->GetPageWidth(pcur + 1);
							float ch2 = m_doc->GetPageHeight(pcur + 1);
							if (ch < ch2) ch = ch2;

							cell->page_left = pcur;
							cell->page_right = pcur + 1;
							pcur += 2;
						}
						else
						{
							cell->page_left = pcur++;
							cell->page_right = -1;
						}
					}
					else
					{
						cell->page_left = pcur++;
						cell->page_right = -1;
					}
					switch (m_scale_mode)
					{
					case SCALE_SAME_WIDTH:
						scalew = minscalew / cw;
						cell->scale = scalew / m_scale_min;
						break;
					case SCALE_SAME_HEIGHT:
						scaleh = minscaleh / ch;
						cell->scale = scaleh / m_scale_min;
						break;
					case SCALE_FIT:
						scalew = (m_vw - m_page_gap) / cw;
						scaleh = (m_vh - m_page_gap) / ch;
						cell->scale = ((scalew > scaleh) ? scaleh : scalew) / m_scale_min;
						break;
					default:
						cell->scale = 1;
						break;
					}
					cell->left = m_layw;
					int cellw = (int)(cw * scale * cell->scale) + m_page_gap;
					int cellh = (int)(ch * scale * cell->scale) + m_page_gap;
					int x = m_page_gap >> 1;
					int y = m_page_gap >> 1;
					if (cellw < m_vw) { x = (m_vw - cellw) >> 1; cellw = m_vw; }
					switch (m_align_type)
					{
					case ALIGN_TOP:
						if (cellh < m_vh) { cellh = m_vh; }
						break;
					case ALIGN_BOTTOM:
						if (cellh < m_vh) { y = (m_vh - cellh) - (m_page_gap >> 1); cellh = m_vh; }
						break;
					default:
						if (cellh < m_vh) { y = (m_vh - cellh) >> 1; cellh = m_vh; }
						break;
					}
					cell->right = cell->left + cellw;
					DXPage *pleft = m_pages + cell->page_left;
					if (pleft->dx_layout(m_layw + x, y, scale * cell->scale) && !zoom) pleft->dx_alloc();
					if (cell->page_right >= 0)
					{
						DXPage *pright = m_pages + cell->page_right;
						if (pright->dx_layout(pleft->GetRight(), y, scale * cell->scale) && !zoom) pright->dx_alloc();
					}
					m_layw = cell->right;
					if (m_layh < cellh) m_layh = cellh;
				}
			}
			void layout_rtol(float scale, bool zoom, bool *para, int para_cnt)
			{
				if (m_vw <= 0 || m_vh <= 0) return;
				if (m_vw <= 0 || m_vh <= 0) return;
				float maxw = 0;
				float maxh = 0;
				int minscalew = 0x40000000;
				int minscaleh = 0x40000000;
				int pcur = 0;
				int ccnt = 0;
				boolean last_dual = false;
				while (pcur < m_page_cnt)
				{
					float cw = m_doc->GetPageWidth(pcur);
					float ch = m_doc->GetPageHeight(pcur);
					if (dual_at(para, para_cnt, ccnt))
					{
						if (pcur < m_page_cnt - 1)
						{
							cw += m_doc->GetPageWidth(pcur + 1);
							float ch2 = m_doc->GetPageHeight(pcur + 1);
							if (ch < ch2) ch = ch2;
							pcur += 2;
							if (pcur == m_page_cnt) last_dual = true;
						}
						else pcur++;
					}
					else pcur++;
					if (maxw < cw) maxw = cw;
					if (maxh < ch) maxh = ch;
					float scalew = (m_vw - m_page_gap) / cw;
					float scaleh = (m_vh - m_page_gap) / ch;
					if (scalew > scaleh) scalew = scaleh;
					cw *= scalew;
					ch *= scalew;
					if (minscalew > (int)cw) minscalew = (int)cw;
					if (minscaleh > (int)ch) minscaleh = (int)ch;
					ccnt++;
				}

				bool changed = (!m_cells || m_cells_cnt != ccnt);
				if (changed)
				{
					if (m_cells) free(m_cells);
					m_cells = (PDFCell *)calloc(ccnt, sizeof(PDFCell));
					m_cells_cnt = ccnt;
				}
				m_scale_min = (float)(m_vw - m_page_gap) / maxw;
				float scalew;
				float scaleh = (float)(m_vh - m_page_gap) / maxh;
				if (m_scale_min > scaleh) m_scale_min = scaleh;
				float max_scale = m_scale_min * m_max_zoom;
				if (scale < m_scale_min) scale = m_scale_min;
				if (scale > max_scale) scale = max_scale;
				//if(m_scale == scale) return;
				m_scale = scale;
				m_layw = 0;
				m_layh = 0;
				pcur = m_page_cnt - 1;
				for (int ccur = 0; ccur < ccnt; ccur++)
				{
					float cw = m_doc->GetPageWidth(pcur);
					float ch = m_doc->GetPageHeight(pcur);
					PDFCell *cell = m_cells + ccur;
					if (dual_at(para, para_cnt, ccnt - ccur - 1))
					{
						if (pcur > 0 || last_dual)
						{
							last_dual = false;

							cw += m_doc->GetPageWidth(pcur - 1);
							float ch2 = m_doc->GetPageHeight(pcur - 1);
							if (ch < ch2) ch = ch2;

							cell->page_left = pcur - 1;
							cell->page_right = pcur;
							pcur -= 2;
						}
						else
						{
							cell->page_left = pcur--;
							cell->page_right = -1;
						}
					}
					else
					{
						cell->page_left = pcur--;
						cell->page_right = -1;
					}
					switch (m_scale_mode)
					{
					case SCALE_SAME_WIDTH:
						scalew = minscalew / cw;
						cell->scale = scalew / m_scale_min;
						break;
					case SCALE_SAME_HEIGHT:
						scaleh = minscaleh / ch;
						cell->scale = scaleh / m_scale_min;
						break;
					case SCALE_FIT:
						scalew = (m_vw - m_page_gap) / cw;
						scaleh = (m_vh - m_page_gap) / ch;
						cell->scale = ((scalew > scaleh) ? scaleh : scalew) / m_scale_min;
						break;
					default:
						cell->scale = 1;
						break;
					}
					cell->left = m_layw;
					int cellw = (int)(cw * scale * cell->scale) + m_page_gap;
					int cellh = (int)(ch * scale * cell->scale) + m_page_gap;
					int x = m_page_gap >> 1;
					int y = m_page_gap >> 1;
					if (cellw < m_vw) { x = (m_vw - cellw) >> 1; cellw = m_vw; }
					switch (m_align_type)
					{
					case ALIGN_TOP:
						if (cellh < m_vh) { cellh = m_vh; }
						break;
					case ALIGN_BOTTOM:
						if (cellh < m_vh) { y = (m_vh - cellh) - (m_page_gap >> 1); cellh = m_vh; }
						break;
					default:
						if (cellh < m_vh) { y = (m_vh - cellh) >> 1; cellh = m_vh; }
						break;
					}
					cell->right = cell->left + cellw;
					DXPage *pleft = m_pages + cell->page_left;
					if (pleft->dx_layout(m_layw + x, y, scale * cell->scale) && !zoom) pleft->dx_alloc();
					if (cell->page_right >= 0)
					{
						DXPage *pright = m_pages + cell->page_right;
						if (pright->dx_layout(pleft->GetRight(), y, scale * cell->scale) && !zoom) pright->dx_alloc();
					}
					m_layw = cell->right;
					if (m_layh < cellh) m_layh = cellh;
				}
			}
		public:
			virtual void vLayout(float scale, bool zoom)
			{
				m_locker.lock();
				if (m_vw > m_vh)//landscape
				{
					if (!m_rtol) layout_ltor(scale, zoom, m_horz_dual, m_horz_dual_cnt);
					else layout_rtol(scale, zoom, m_horz_dual, m_horz_dual_cnt);
				}
				else//portrait
				{
					if (!m_rtol) layout_ltor(scale, zoom, m_vert_dual, m_vert_dual_cnt);
					else layout_rtol(scale, zoom, m_vert_dual, m_vert_dual_cnt);
				}
				m_locker.unlock();
			}
			virtual bool vFling(float vx, float vy)
			{
				if (!m_cells) return false;
				vEndScroll();
				int x = vGetX();
				int y = vGetY();
				if (vx < 0) x++;
				else x--;
				int endx = x - (int)vx;
				int endy = y - (int)vy;
				if (endx > m_layw - m_vw) endx = m_layw - m_vw;
				if (endx < 0) endx = 0;
				if (endy > m_layh - m_vh) endy = m_layh - m_vh;
				if (endy < 0) endy = 0;
				int cell1 = get_cell(x);
				int cell2 = get_cell(endx);
				if (cell2 > cell1) cell2 = cell1 + 1;
				if (cell2 < cell1) cell2 = cell1 - 1;
				//vScrollAbort();
				if (cell1 < cell2) {
					if (cell2 == m_cells_cnt)
						do_scroll(x, y, m_cells[cell2 - 1].right - m_vw - x, endy - y);
					else {
						PDFCell *cell = m_cells + cell1;
						if (x < cell->right - m_vw)
							do_scroll(x, y, cell->right - m_vw - x, endy - y);
						else
							do_scroll(x, y, m_cells[cell2].left - x, endy - y);
					}
				}
				else if (cell1 > cell2) {
					if (cell2 < 0)
						do_scroll(x, y, -x, 0);
					else {
						PDFCell *cell = m_cells + cell1;
						if (x > cell->left)
							do_scroll(x, y, cell->left - x, endy - y);
						else
							do_scroll(x, y, m_cells[cell2].right - m_vw - x, endy - y);
					}
				}
				else {
					PDFCell *cell = m_cells + cell2;
					if (endx + m_vw > cell->right) {
						if (endx + (m_vw >> 1) > cell->right) {
							cell2++;
							if (cell2 == m_cells_cnt)
								do_scroll(x, y, m_cells[cell2 - 1].right - m_vw - x, endy - y);
							else
								do_scroll(x, y, m_cells[cell2].left - x, endy - y);
						}
						else
							do_scroll(x, y, m_cells[cell2].right - m_vw - x, endy - y);
					}
					else {
						do_scroll(x, y, endx - x, endy - y);
					}
				}
				return true;
			}
			virtual void vMoveEnd()
			{
				int ccur = 0;
				int x = vGetX();
				int y = vGetY();
				while (ccur < m_cells_cnt)
				{
					PDFCell *cell = m_cells + ccur;
					if (x < cell->right)
					{
						m_scroller.abortAnimation();
						m_scroller.forceFinished(true);
						if (x <= cell->right - m_vw)
						{
						}
						else if (cell->right - x > (m_vw >> 1))
						{
							m_scroller.startScroll(x, y, cell->right - x - m_vw, 0);
						}
						else if (ccur < m_cells_cnt - 1)
						{
							m_scroller.startScroll(x, y, cell->right - x, 0);
						}
						else
						{
							m_scroller.startScroll(x, y, cell->right - x - m_vw, 0);
						}
						break;
					}
					ccur++;
				}
			}
			void vGotoPage(int pageno)
			{
				if (!m_pages || !m_doc || m_vw <= 0 || m_vh <= 0) return;
				vEndScroll();
				int ccur = 0;
				while (ccur < m_cells_cnt)
				{
					PDFCell *cell = m_cells + ccur;
					if (pageno == cell->page_left || pageno == cell->page_right)
					{
						int left = m_cells[ccur].left;
						int w = m_cells[ccur].right - left;
						int x = left + ((w - m_vw) >> 1);
						m_scroller.setFinalX(x);
						break;
					}
					ccur++;
				}
			}
		};
	}
}
