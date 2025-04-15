#pragma once
#include "DXCommon.h"
namespace RDPDFLib
{
	namespace view
	{
		class DXBlock;
		class DXFinder;
		interface IDXThreadCallback
		{
			virtual void onVCacheRendered(DXBlock *blk) = 0;
			virtual void onVFound(int ret) = 0;
		};
		class DXThread
		{
		public:
			DXThread(Windows::UI::Core::CoreDispatcher ^disp, IDXThreadCallback *callback)
			{
				queue_cur = 0;
				queue_next = 0;
				m_run = false;
				m_disp = disp;
				m_callback = callback;
			}
			~DXThread()
			{
				destroy();
				m_disp = nullptr;
				m_callback = NULL;
			}
			bool start();
			void destroy()
			{
				if (m_run)
				{
					post_msg(0xFFFFFFFF, NULL, NULL);
					m_eve.wait();
					queue_cur = queue_next = 0;
					m_run = false;
				}
			}
			void render_start(DXBlock *blk);
			bool render_end(DXBlock *blk);
			inline void find_start(DXFinder *finder)
			{
				post_msg(3, finder, NULL);
			}
			inline bool is_run() { return m_run; }
		protected:
			struct QUEUE_NODE
			{
				unsigned mid;
				void *para1;
				void *para2;
			};
			QUEUE_NODE queue_items[4096];
			int queue_cur;
			int queue_next;
			bool m_run;
			DXEvent queue_event;
			DXEvent m_eve;
			DXLocker m_locker;
			Windows::UI::Core::CoreDispatcher ^m_disp;
			IDXThreadCallback *m_callback;

			void post_msg(unsigned int mid, void *para1, void *para2)
			{
				m_locker.lock();
				QUEUE_NODE *item = queue_items + queue_next;
				item->mid = mid;
				item->para1 = para1;
				item->para2 = para2;
				int next = queue_next;
				queue_next = (queue_next + 1) & 4095;
				if (queue_cur == next) queue_event.notify();
				m_locker.unlock();
			}
			void get_msg(QUEUE_NODE &ret)
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
				QUEUE_NODE *item = queue_items + queue_cur;
				item->mid = 0;
				item->para1 = NULL;
				item->para2 = NULL;
				queue_cur = (queue_cur + 1) & 4095;
				m_locker.unlock();
			}
		};
	}
}
