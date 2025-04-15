#pragma once
#include "PDFCore.h"
#include "PDFLayout.h"
#include "VUtil.h"
#include <Windows.h>
using namespace RDPDFLib::pdf;
using namespace RDPDFLib::view;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace RDPDFLib
{
	namespace reader
	{
		public enum class PDF_LAYOUT_MODE
		{
			layout_unknown = 0,
			layout_vert,
			layout_horz,
			layout_dual,
			layout_dual_continous,
			layout_thumb
		};
		public ref class PDFLayoutView sealed : public Panel, IVPanel
		{
		public:
			PDFLayoutView() :Panel()
			{
				Windows::Graphics::Display::DisplayInformation^ disp = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
				m_dpi = disp->LogicalDpi;
				m_doc = nullptr;
				m_layout = NULL;
			}
			virtual ~PDFLayoutView()
			{
				Close();
			}
			bool Open(PDFDoc^ doc)
			{
				if (!doc) return false;
				m_doc = doc;
				return true;
				SetView(PDF_LAYOUT_MODE::layout_vert);
			}
			void SetView(PDF_LAYOUT_MODE vmode)
			{
				switch (vmode)
				{
				case PDF_LAYOUT_MODE::layout_vert:
					m_layout = new PDFLayoutVert(0);
					break;
				default:
					m_layout = new PDFLayoutVert(0);
					break;
				}
				m_layout->vOpen(m_doc, this, 4);
			}
			void Close()
			{
				if (!m_doc) return;
				if (m_layout) delete m_layout;
				m_doc = nullptr;
			}
		public://implement interface
			virtual void vpShowBlock(Image^ img);
			virtual void vpRemoveBlock(Image^ img);
		protected:
			Windows::Foundation::Size MeasureOverride(Windows::Foundation::Size size) override
			{
				Windows::Foundation::Size ret = size;
				if (m_layout)
				{
					ret.Width = m_layout->vGetLayoutW();
					ret.Height = m_layout->vGetLayoutH();
				}
				else
				{
					ret.Width = size.Width;
					ret.Height = 0;
				}
				return ret;
			}
			Windows::Foundation::Size ArrangeOverride(Windows::Foundation::Size finalSize) override
			{
				return finalSize;
			}
		private:
			PDFDoc^ m_doc;
			float m_dpi;
			PDFLayout* m_layout;
		};
	}
}