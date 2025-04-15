#pragma once
#include "PDFCore.h"
#include "DXDevice.h"
#include "DXLayout.h"
#include "DXSel.h"
#include "DXOPStack.h"
namespace RDPDFLib
{
	namespace view
	{
		public interface class IDXViewCallback
		{
			/**
			 * call when page changed.
			 * @param pageno
			 */
			void OnPDFPageModified(int pageno);

			/**
			 * call when page scrolling.
			 * @param pageno
			 */
			void OnPDFPageChanged(int pageno);

			/**
			 * call when annotation tapped.
			 * @param pno
			 * @param annot
			 */
			void OnPDFAnnotTapped(int pno, pdf::PDFAnnot ^annot);

			/**
			 * call when blank tapped on page, this mean not annotation tapped.
			 */
			void OnPDFBlankTapped();

			/**
			 * call select status end.
			 * @param text selected text string
			 */
			void OnPDFSelectEnd(Platform::String ^text);

			void OnPDFOpenURI(Platform::String ^uri);
			void OnPDFOpenJS(Platform::String ^js);
			void OnPDFOpenMovie(pdf::PDFAnnot ^annot, Platform::String ^name);
			void OnPDFOpenSound(pdf::PDFAnnot ^annot, Platform::String^name);
			void OnPDFOpenAttachment(pdf::PDFAnnot ^annot, Platform::String^name);
			void OnPDFOpen3D(pdf::PDFAnnot ^annot, Platform::String^name);

			/**
			 * call when zoom start.
			 */
			void OnPDFZoomStart();

			/**
			 * call when zoom end
			 */
			void OnPDFZoomEnd();
			bool OnPDFDoubleTapped(float x, float y);
			void OnPDFLongPressed(float x, float y);

			/**
			 * call when search finished. each search shall call back each time.
			 * @param found
			 */
			void OnPDFSearchFinished(bool found);

			/**
			 * call when page displayed on screen.
			 * @param canvas
			 * @param vpage
			 */
			void OnPDFPageDisplayed(Windows::UI::Xaml::Controls::Canvas ^canvas, int pageno, int x, int y, float scale);
			/**
			 * call when page is rendered by backing thread.
			 * @param vpage
			 */
			void OnPDFPageRendered(int pageno);
		};
		class DXPointerRec
		{
		public:
			DXPointerRec()
			{
				m_pts = NULL;
				m_pts_cnt = 0;
				m_pts_max = 0;
				m_hold = false;
			}
			~DXPointerRec()
			{
				if (m_pts) free(m_pts);
				m_pts = NULL;
				m_pts_cnt = 0;
				m_pts_max = 0;
				m_hold = false;
			}
			inline void add(unsigned int id)
			{
				if (m_pts_cnt >= m_pts_max)
				{
					m_pts_max += 32;
					m_pts = (unsigned int *)realloc(m_pts, sizeof(unsigned int) * m_pts_max);
				}
				m_pts[m_pts_cnt++] = id;
				if (m_pts_cnt == 1)
					m_hold = true;
				else m_hold = false;
			}
			inline bool find(unsigned int id)
			{
				unsigned int *cur = m_pts;
				unsigned int *end = m_pts + m_pts_cnt;
				while (cur < end)
				{
					if (*cur++ == id) return true;
				}
				return false;
			}
			inline void remove(unsigned int id)
			{
				unsigned int *cur = m_pts;
				unsigned int *end = m_pts + m_pts_cnt;
				while (cur < end)
				{
					if (*cur++ == id)
					{
						while (cur < end)
						{
							cur[-1] = cur[0];
							cur++;
						}
						m_pts_cnt--;
						m_hold = false;
						return;
					}
				}
				m_hold = false;
			}
			unsigned int *m_pts;
			int m_pts_cnt;
			int m_pts_max;
			bool m_hold;
		};
		class DXPageSet
		{
		public:
			DXPageSet(int max_len)
			{
				pages = (DXPage **)malloc(sizeof(DXPage *) * max_len);
				pages_cnt = 0;
			}
			~DXPageSet()
			{
				free(pages);
			}
			void Insert(DXPage *vpage)
			{
				int cur = 0;
				for (cur = 0; cur < pages_cnt; cur++)
				{
					if (pages[cur] == vpage) return;
				}
				pages[cur] = vpage;
				pages_cnt++;
			}
			DXPage **pages;
			int pages_cnt;
		};
		public enum class DXOPCODE
		{
			OP_START = 0,
			OP_END = 1,
			OP_CANCEL = 2,
		};
		public ref class DXView sealed : public IDXDeviceCallback, IDXLayoutCallback
		{
		public:
			DXView(Windows::UI::Xaml::Controls::SwapChainPanel^ panel, Windows::UI::Xaml::Controls::Canvas ^canvas);
			virtual void dx_update();
			virtual bool dx_render();
			virtual void onVPageChanged(int pageno);
			virtual void onVFound(bool found);
			virtual void onVCacheRendered(int pageno);

			void PDFOpen(pdf::PDFDoc ^doc, IDXViewCallback ^callback, int page_gap);
			void PDFSetView(int view_mode);
			void PDFClose();
			void PDFEndAnnot();
			void PDFPerformAnnot();
			void PDFFindStart(String ^key, boolean match_case, boolean whole_word);
			void PDFFind(int dir);
			void PDFFindEnd();
			bool PDFSetSelMarkup(int type);
			void PDFGotoPage(int pageno);
			bool PDFUndo();
			bool PDFRedo();
			inline bool PDFCanUndo() { return m_opstack.can_undo(); }
			inline bool PDFCanRedo() { return m_opstack.can_redo(); }
			void PDFSetInk(DXOPCODE code);
			void PDFSetRect(DXOPCODE code);
			void PDFSetEllipse(DXOPCODE code);
			void PDFSetSelect(DXOPCODE code);
			void PDFSetNote(DXOPCODE code);
			void PDFSetLine(DXOPCODE code);
			void PDFSetStamp(DXOPCODE code);
		private:
			~DXView()
			{
				PDFClose();
				m_device.StopRenderLoop();
				m_page_gap = 4;
			}
			void dx_visibility_changed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
			void dx_dpi_changed(Windows::Graphics::Display::DisplayInformation^ sender, Object^ args);
			void dx_orientation_changed(Windows::Graphics::Display::DisplayInformation^ sender, Object^ args);
			void dx_invalidate(Windows::Graphics::Display::DisplayInformation^ sender, Object^ args);

			void onVCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args);
			void onVSizeChanged(Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
			void onVTapped(Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^e);
			void onVDoubleTapped(Object^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^e);
			void onVManipulationStarted(Object ^sender, Windows::UI::Xaml::Input::ManipulationStartedRoutedEventArgs ^ e);
			void onVManipulationDelta(Object ^sender, Windows::UI::Xaml::Input::ManipulationDeltaRoutedEventArgs ^ e);
			void onVManipulationCompleted(Object ^sender, Windows::UI::Xaml::Input::ManipulationCompletedRoutedEventArgs ^ e);
			void onVManipulationInertiaStarting(Object ^sender, Windows::UI::Xaml::Input::ManipulationInertiaStartingRoutedEventArgs ^e);
			void onVDrawCanvas();
			void onVTouchDown(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			void onVTouchMove(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			void onVTouchUp(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			void onVWheelChanged(Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^e);
			pdf::PDFDoc ^m_doc;
			IDXViewCallback ^m_listener;
			Windows::UI::Xaml::Controls::Canvas ^m_canvas;
			DXDevice m_device;
			DXLayout *m_layout;
			int m_page_gap;
			enum _DXSTATUS
			{
				STA_NONE = 0,
				STA_ZOOM = 1,
				STA_SELECT = 2,
				STA_INK = 3,
				STA_RECT = 4,
				STA_ELLIPSE = 5,
				STA_NOTE = 6,
				STA_LINE = 7,
				STA_STAMP = 8,
				STA_ANNOT = 100,
			};
			_DXSTATUS m_status;

			bool staNoneDrawCanvas();
			bool staNoneTouchDown(float x, float y);
			bool staNoneTouchMove(float x, float y);
			bool staNoneTouchUp(float x, float y);

			bool staSelectDrawCanvas();
			bool staSelectTouchDown(float x, float y);
			bool staSelectTouchMove(float x, float y);
			bool staSelectTouchUp(float x, float y);

			bool staAnnotDrawCanvas();
			bool staAnnotTouchDown(float x, float y);
			bool staAnnotTouchMove(float x, float y);
			bool staAnnotTouchUp(float x, float y);

			bool staNoteDrawCanvas();
			bool staNoteTouchDown(float x, float y);
			bool staNoteTouchMove(float x, float y);
			bool staNoteTouchUp(float x, float y);

			bool staInkDrawCanvas();
			bool staInkTouchDown(float x, float y);
			bool staInkTouchMove(float x, float y);
			bool staInkTouchUp(float x, float y);

			bool staLineDrawCanvas();
			bool staLineTouchDown(float x, float y);
			bool staLineTouchMove(float x, float y);
			bool staLineTouchUp(float x, float y);

			bool staRectDrawCanvas();
			bool staRectTouchDown(float x, float y);
			bool staRectTouchMove(float x, float y);
			bool staRectTouchUp(float x, float y);

			bool staEllipseDrawCanvas();
			bool staEllipseTouchDown(float x, float y);
			bool staEllipseTouchMove(float x, float y);
			bool staEllipseTouchUp(float x, float y);

			bool staStampDrawCanvas();
			bool staStampTouchDown(float x, float y);
			bool staStampTouchMove(float x, float y);
			bool staStampTouchUp(float x, float y);

			DXPointerRec m_pts;
			float m_zoom_scale;
			float m_hold_x;
			float m_hold_y;
			int m_hold_docx;
			int m_hold_docy;
			PDFPos m_annot_pos;
			DXPage *m_annot_page;
			pdf::PDFAnnot ^m_annot;
			pdf::PDFRect m_annot_rect;
			pdf::PDFRect m_annot_rect0;
			float m_annot_x0;
			float m_annot_y0;
			DXSel *m_sel;
			struct _DXNOTE
			{
				DXPage *page;
				int index;
			};
			_DXNOTE *m_notes;
			int m_notes_cnt;
			pdf::PDFInk ^m_ink;
			float *m_rects;
			int m_rects_cnt;
			DXOPStack m_opstack;
		};
	}
}