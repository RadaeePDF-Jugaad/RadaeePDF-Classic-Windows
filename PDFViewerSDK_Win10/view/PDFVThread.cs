using System.Threading;
using System;
using Windows.Foundation;
using Windows.UI.Core;
using Windows.System.Threading;
using System.Threading.Tasks;

namespace PDFViewerSDK_Win10
{
    namespace view
    {
        public struct PDFVUIHandler
        {
            public CoreDispatcher disp;
            public DispatchedHandler render;
            public DispatchedHandler finder;
            public DispatchedHandler timer;
        }
        public class PDFVThread
        {
            struct QUEUE_NODE
            {
                public uint mid;
                public object para1;
                public object para2;
            }
            PDFVUIHandler m_ui;
            QUEUE_NODE[] queue_items = new QUEUE_NODE[128];
            int queue_cur;
            int queue_next;
            bool m_notified;
            ThreadPoolTimer m_timer = null;
            bool m_run;
            ManualResetEventSlim queue_event;
            ManualResetEventSlim m_eve;
            public PDFVThread()
            {
                queue_cur = 0;
                queue_next = 0;
                m_notified = false;
                m_run = false;
            }
            protected void onRun(object obj)
            {
                QUEUE_NODE node;
                while (true)
                {
                    node = get_msg();
                    if (node.mid == 0xFFFFFFFF) break;
                    switch (node.mid)
                    {
                        case 1:
                            ((PDFVCache)node.para1).Render();
                            m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, m_ui.render);
                            break;
                        case 2:
                            ((PDFVCache)node.para1).Clear();
                            break;
                        case 3:
                            ((PDFVFinder)node.para1).find();
                            m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, m_ui.finder);
                            break;
                        case 4:
                            ((PDFVThumb)node.para1).Render();
                            m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, m_ui.render);
                            break;
                        case 5:
                            ((PDFVThumb)node.para1).Clear();
                            break;
                    }
                    node.para1 = null;
                    node.para2 = null;
                }
                m_eve.Set();
            }
            private void onTimer(ThreadPoolTimer sender)
            {
                m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, m_ui.timer);
            }
            public bool create(PDFVUIHandler hand)
            {
                if (m_run) return true;
                queue_event = new ManualResetEventSlim();
                m_eve = new ManualResetEventSlim();
                m_ui = hand;
                queue_event.Reset();
                m_eve.Reset();
                m_run = true;
                m_notified = false;
                Windows.System.Threading.ThreadPool.RunAsync(new WorkItemHandler(onRun), WorkItemPriority.Normal, WorkItemOptions.TimeSliced);
                m_timer = ThreadPoolTimer.CreatePeriodicTimer(new TimerElapsedHandler(onTimer), TimeSpan.FromMilliseconds(100));
                return true;
            }
            public void destroy()
            {
                if (m_run)
                {
                    post_msg(0xFFFFFFFF, null, null);
                    m_eve.Wait();
                    queue_event.Dispose();
                    m_eve.Dispose();
                    queue_cur = queue_next = 0;
                    m_notified = false;
                    m_run = false;
                    if (m_timer != null)
                        m_timer.Cancel();
                    m_timer = null;
                }
            }
            public void start_render(PDFVPage page, bool is_thumb)
            {
                switch (page.RenderPrepare(is_thumb))
                {
                    case 1: break;
                    case 2: end_render(page); start_render(page, is_thumb); break;
                    default: post_msg(1, page.m_cache, page); break;
                }
            }
            public void end_render(PDFVPage page)
            {
                PDFVCache cache = page.CancelRender();
                if (cache != null)
                    post_msg(2, cache, null);
            }
            public void start_thumb(PDFVPage page, bool is_thumb)
            {
                switch (page.RenderPrepare(is_thumb))
                {
                    case 1: break;
                    case 2: end_thumb(page); start_thumb(page, is_thumb); break;
                    default: post_msg(4, page.m_thumb, page); break;
                }
            }
            public void end_thumb(PDFVPage page)
            {
                PDFVThumb cache = page.CancelThumb();
                if (cache != null)
                    post_msg(5, cache, null);
            }
            public void start_find(PDFVFinder finder)
            {
                post_msg(3, finder, null);
            }
            private void post_msg(uint mid, object para1, object para2)
            {
                lock (this)
                {
                    QUEUE_NODE item;
                    item.mid = mid;
                    item.para1 = para1;
                    item.para2 = para2;
                    queue_items[queue_next] = item;
                    int next = queue_next;
                    queue_next = (queue_next + 1) & 127;
                    if (queue_cur == next)
                    {
                        if (!m_notified)
                        {
                            queue_event.Set();
                            m_notified = true;
                        }
                    }
                }
            }
            private QUEUE_NODE get_msg()
            {
                bool wait_it = false;
                lock (this)
                {
                    if (m_notified)
                    {
                        queue_event.Reset();
                        m_notified = false;
                    }
                    else if (queue_cur == queue_next)
                        wait_it = true;
                }
                if (wait_it) queue_event.Wait();
                QUEUE_NODE ret;
                lock (this)
                {
                    //queue_event.Reset();
                    ret = queue_items[queue_cur];
                    queue_items[queue_cur].mid = 0;
                    queue_items[queue_cur].para1 = null;
                    queue_items[queue_cur].para2 = null;
                    queue_cur = (queue_cur + 1) & 127;
                }
                return ret;
            }
        }
    }
}