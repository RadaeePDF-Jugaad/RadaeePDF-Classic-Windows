#pragma once
#include <Windows.h>
using namespace Windows::UI::Core;
namespace RDPDFLib
{
	namespace view
	{
		class CRDVBlk;
		class CRDVFinder;
		interface class IVCallback;
		class CRDLocker
		{
		public:
			CRDLocker()
			{
				InitializeCriticalSectionEx(&csLocker, 0, CRITICAL_SECTION_NO_DEBUG_INFO);
			}
			~CRDLocker()
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

		class CRDEvent
		{
		public:
			CRDEvent()
			{
				m_event = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
			}
			~CRDEvent()
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
		class CRDVThread
		{
		public:
			CRDVThread()
			{
				queue_cur = 0;
				queue_next = 0;
				m_hThread = NULL;
				queue_event.reset();
			}
			~CRDVThread()
			{
				destroy();
			}
			inline bool is_run()
			{
				return m_hThread != NULL;
			}
			inline void start()
			{
				if (m_hThread) return;
				m_hThread = CreateThread(NULL, 0, ProcWork, this, NORMAL_PRIORITY_CLASS, NULL);
			}
			void destroy()
			{
				if (m_hThread)
				{
					post_msg(0xFFFFFFFF, NULL);
					::WaitForSingleObject(m_hThread, INFINITE);
					CloseHandle(m_hThread);
					m_hThread = NULL;
					queue_cur = queue_next = 0;
					m_disp = nullptr;
					m_render = nullptr;
					m_finder = nullptr;
					m_destroy = nullptr;
				}
			}
			void render_start(CRDVBlk* blk);
			bool render_end(IVCallback^ canvas, CRDVBlk* blk);
			CRDVBlk *render_end2(IVCallback^ canvas, CRDVBlk* blk);
			inline void find_start(CRDVFinder* finder)
			{
				post_msg(2, finder);
			}
			inline void set_callback(CoreDispatcher ^disp, DispatchedHandler ^ render, DispatchedHandler ^finder, DispatchedHandler ^destroy)
			{
				m_disp = disp;
				m_render = render;
				m_finder = finder;
				m_destroy = destroy;
			}
		protected:
			static DWORD WINAPI ProcWork(void* para);
			struct QUEUE_NODE
			{
				unsigned int mid;
				void* para1;
			};
			HANDLE m_hThread;
			CoreDispatcher^ m_disp;
			DispatchedHandler^ m_render;
			DispatchedHandler^ m_finder;
			DispatchedHandler^ m_destroy;
			QUEUE_NODE queue_items[4096];
			int queue_cur;
			int queue_next;
			CRDEvent queue_event;
			CRDLocker m_locker;
			void post_msg(unsigned int mid, void* para1)
			{
				m_locker.lock();
				QUEUE_NODE* item = queue_items + queue_next;
				item->mid = mid;
				item->para1 = para1;
				int next = queue_next;
				queue_next = (queue_next + 1) & 4095;
				if (queue_cur == next) queue_event.notify();
				m_locker.unlock();
			}
			bool get_msg(QUEUE_NODE& ret)
			{
				m_locker.lock();
				while (queue_cur == queue_next)
				{
					m_locker.unlock();
					queue_event.wait();
					m_locker.lock();
				}
				//queue_event.Reset();
				ret = queue_items[queue_cur];
				QUEUE_NODE* item = queue_items + queue_cur;
				item->mid = 0;
				item->para1 = NULL;
				queue_cur = (queue_cur + 1) & 4095;
				m_locker.unlock();
				return ret.mid != 0xFFFFFFFF;
			}
		};
	}
}