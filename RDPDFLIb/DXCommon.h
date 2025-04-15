#pragma once
using namespace Windows::UI;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Shapes;
using namespace Windows::UI::Xaml::Media;
namespace RDPDFLib
{
	namespace view
	{
		class DXCanvas
		{
		public:
			static void fill_rect(Canvas ^canvas, const Color &clr, float left, float top, float right, float bottom)
			{
				Rectangle ^rect1 = ref new Rectangle();
				rect1->Fill = ref new SolidColorBrush(clr);
				rect1->SetValue(Canvas::LeftProperty, left);
				rect1->SetValue(Canvas::TopProperty, top);
				rect1->Width = (right - left);
				rect1->Height = (bottom - top);
				canvas->Children->Append(rect1);
			}
			static void draw_rect(Canvas ^canvas, const Color &clr, float left, float top, float right, float bottom, float width)
			{
				Rectangle ^rect1 = ref new Rectangle();
				rect1->Stroke = ref new SolidColorBrush(clr);
				rect1->StrokeThickness = width;
				rect1->SetValue(Canvas::LeftProperty, left);
				rect1->SetValue(Canvas::TopProperty, top);
				rect1->Width = right - left;
				rect1->Height = bottom - top;
				canvas->Children->Append(rect1);
			}
			static void draw_lines(Canvas ^canvas, const Color &clr, float *pts, int cnt, float width)
			{
				SolidColorBrush ^br = ref new SolidColorBrush(clr);
				cnt <<= 1;
				float w = width * 0.5f;
				float *cur = pts;
				float *end = cur + cnt;
				while(cur < end)
				{
					Line ^line = ref new Line();
					line->StrokeThickness = width;
					line->Stroke = br;
					line->SetValue(Line::X1Property, cur[0]);
					line->SetValue(Line::Y1Property, cur[1]);
					line->SetValue(Line::X2Property, cur[2]);
					line->SetValue(Line::Y2Property, cur[3]);
					canvas->Children->Append(line);
					cur += 4;
				}
			}
		};
		class DXLocker
		{
		public:
			DXLocker()
			{
				InitializeCriticalSectionEx(&csLocker, 0, CRITICAL_SECTION_NO_DEBUG_INFO);
			}
			~DXLocker()
			{
				DeleteCriticalSection(&csLocker);
			}
			inline void lock()
			{
				EnterCriticalSection(&csLocker);
			}
			inline void unlock()
			{
				LeaveCriticalSection(&csLocker);
			}
		protected:
			CRITICAL_SECTION csLocker;
		};
		class DXEvent
		{
		public:
			DXEvent()
			{
				m_event = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
			}
			~DXEvent()
			{
				if (m_event)
				{
					while (!CloseHandle(m_event));
					m_event = NULL;
				}
			}
			inline void reset()
			{
				if (!m_event) return;
				while (!ResetEvent(m_event));
			}
			inline void notify()
			{
				if (!m_event) return;
				while (!SetEvent(m_event));
			}
			inline void wait()
			{
				if (!m_event) return;
				while (WaitForSingleObjectEx(m_event, -1, FALSE) != WAIT_OBJECT_0);
			}
		protected:
			HANDLE m_event;
		};
	}
}