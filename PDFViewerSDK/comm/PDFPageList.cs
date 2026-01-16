using RDPDFLib.pdf;
using RDPDFReader;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.System.Threading;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

namespace RDPDFReader
{
    public struct PDFPageUIHandler
    {
        public CoreDispatcher disp;
        public PDFPageList list;
    }
    public class PDFPageThread
    {
        struct QUEUE_NODE
        {
            public uint mid;
            public object para1;
            public object para2;
        }
        PDFPageUIHandler m_ui;
        QUEUE_NODE[] queue_items = new QUEUE_NODE[128];
        int queue_cur;
        int queue_next;
        bool m_notified;
        bool m_run;
        ManualResetEventSlim queue_event;
        ManualResetEventSlim m_eve;
        public PDFPageThread()
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
                        {
                            UIPageItem item = (UIPageItem)node.para1;
                            item.bk_render();
                            m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, async () =>
                            {
                                m_ui.list.vOnUpdatePage(item);
                            });
                        }
                        break;
                    case 2:
                        {
                            UIPageItem item = (UIPageItem)node.para1;
                            item.bk_clear();
                            m_ui.disp.RunAsync(CoreDispatcherPriority.Normal, async () =>
                            {
                                item.ui_clear();
                            });
                        }
                        break;
                }
                node.para1 = null;
                node.para2 = null;
            }
            m_eve.Set();
        }
        public bool create(PDFPageUIHandler hand)
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
            }
        }
        public void start_render(UIPageItem page)
        {
            if (page.ui_render_start())
                post_msg(1, page, null);
        }
        public void end_render(UIPageItem page)
        {
            if (page.ui_render_end())
                post_msg(2, page, null);
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
    class PDFPageCanvas : Canvas
    {
        public const int CELL_W = 200;
        public const int CELL_H = 200 + 32;
        public const int CELL_GAP = 8;
        private PDFDoc m_doc;
        public PDFPageCanvas(PDFDoc doc) : base()
        {
            m_doc = doc;
        }
        protected override Windows.Foundation.Size MeasureOverride(Windows.Foundation.Size size)
        {
            Windows.Foundation.Size ret = base.MeasureOverride(size);
            double wh = size.Height;
            int wcnt = ((int)size.Width) / (CELL_W + CELL_GAP);
            if (wcnt < 1) wcnt = 1;
            ret.Width = size.Width;
            ret.Height = (CELL_H + CELL_GAP) * ((m_doc.PageCount + wcnt - 1) / wcnt);
            return ret;
        }
        protected override Windows.Foundation.Size ArrangeOverride(Windows.Foundation.Size finalSize)
        {
            Windows.Foundation.Size ret = base.ArrangeOverride(finalSize);
            double wh = finalSize.Height;
            int wcnt = ((int)finalSize.Width) / (CELL_W + CELL_GAP);
            if (wcnt < 1) wcnt = 1;
            ret.Width = finalSize.Width;
            ret.Height = (CELL_H + CELL_GAP) * ((m_doc.PageCount + wcnt - 1) / wcnt);
            return ret;
        }
    }
    public class PDFPageList
    {
        private UIPageItem[] m_items_org;
        private UIPageItem[] m_items;
        private ScrollViewer m_view;
        private PDFPageCanvas m_canvas;
        private PDFPageThread m_thread;
        private PDFDoc m_doc;
        private void vOnUpdateView()
        {
            double offx = m_view.HorizontalOffset;
            double offy = m_view.VerticalOffset;
            double vw = m_view.ActualWidth;
            double vh = m_view.ActualHeight;
            if (vw <= 0 || vh <= 0) return;
            int wcnt = ((int)vw) / (PDFPageCanvas.CELL_W + PDFPageCanvas.CELL_GAP);
            int ibs = ((int)offy) / (PDFPageCanvas.CELL_H + PDFPageCanvas.CELL_GAP);
            int ibe = ((int)(offy + vh)) / (PDFPageCanvas.CELL_H + PDFPageCanvas.CELL_GAP) + wcnt;
            double xcur = (PDFPageCanvas.CELL_GAP >> 1);
            double ycur = ibs * (PDFPageCanvas.CELL_H + PDFPageCanvas.CELL_GAP) + (PDFPageCanvas.CELL_GAP >> 1);

            UIPageItem item;
            int pgno;
            ibs *= wcnt;
            for (pgno = 0; pgno < ibs; pgno++)
            {
                item = m_items[pgno];
                if (item != null)
                {
                    m_thread.end_render(item);
                    m_canvas.Children.Remove(item);
                }
            }
            ibe *= wcnt;
            if (ibe > m_items.Length) ibe = m_items.Length;
            while (pgno < ibe)//layout and rendering?
            {
                item = m_items[pgno];
                if (item.Parent == null)
                {
                    item.ui_setpos(xcur, ycur);
                    m_canvas.Children.Add(item);
                }
                m_thread.start_render(item);
                pgno++;
                if (pgno % wcnt == 0)
                {
                    xcur = (PDFPageCanvas.CELL_GAP >> 1);
                    ycur += (PDFPageCanvas.CELL_H + PDFPageCanvas.CELL_GAP);
                }
                else xcur += (PDFPageCanvas.CELL_W + PDFPageCanvas.CELL_GAP);
            }
            if (ycur < m_view.ActualHeight)
                m_canvas.Height = m_view.ActualHeight;
            while (pgno < m_items.Length)
            {
                item = m_items[pgno];
                if (item != null)
                {
                    m_thread.end_render(item);
                    m_canvas.Children.Remove(item);
                }
                pgno++;
            }
        }
        private void vOnSizeChanged(Object sender, SizeChangedEventArgs e)
        {
            if (m_items == null) return;
            for (int pgno = 0; pgno < m_items.Length; pgno++)
            {
                UIPageItem item = m_items[pgno];
                if (item != null)
                {
                    m_thread.end_render(item);
                    m_canvas.Children.Remove(item);
                }
            }
            vOnUpdateView();
        }
        private void vOnViewChanged(Object sender, ScrollViewerViewChangedEventArgs e)
        {
            vOnUpdateView();
        }
        public void vOnUpdatePage(UIPageItem item)
        {
            item.ui_update();
        }
        private void refresh_pages()
        {
            int count = 0;
            int cur = 0;
            for (cur = 0; cur < m_items_org.Length; cur++)
            {
                UIPageItem item = m_items_org[cur];
                if (!item.m_deleted) count++;
            }
            m_items = new UIPageItem[count];
            int pcur = 0;
            for (cur = 0; cur < m_items_org.Length; cur++)
            {
                UIPageItem item = m_items_org[cur];
                if (!item.m_deleted) m_items[pcur++] = item;
            }
        }
        private void vOnDeleteTapped(Object sender, TappedRoutedEventArgs e)
        {
            Image img = (Image)sender;
            UIPageItem item = (UIPageItem)img.Tag;
            item.m_deleted = true;
            m_view.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => {
                for (int pgno = 0; pgno < m_items.Length; pgno++)
                {
                    UIPageItem icur = m_items[pgno];
                    if (icur != null)
                    {
                        m_thread.end_render(icur);
                        m_canvas.Children.Remove(icur);
                    }
                }
                refresh_pages();
                vOnUpdateView();
            });
        }
        public void PDFOpen(ScrollViewer view, PDFDoc doc)
        {
            m_view = view;
            m_doc = doc;
            int pages_cnt = doc.PageCount;
            m_items = new UIPageItem[pages_cnt];
            m_items_org = new UIPageItem[pages_cnt];
            for (int ip = 0; ip < pages_cnt; ip++)
            {
                UIPageItem item = new UIPageItem();
                item.ui_init(doc, ip, vOnDeleteTapped);
                m_items_org[ip] = item;
                m_items[ip] = item;
            }

            m_canvas = new PDFPageCanvas(m_doc);
            m_view.Content = m_canvas;
            m_view.SizeChanged += vOnSizeChanged;
            m_view.ViewChanged += vOnViewChanged;
            m_thread = new PDFPageThread();
            PDFPageUIHandler hand = new PDFPageUIHandler();
            hand.disp = m_view.Dispatcher;
            hand.list = this;
            m_thread.create(hand);
            Color clr;
            clr.A = 255;
            clr.R = 224;
            clr.G = 224;
            clr.B = 224;
            m_view.Background = new SolidColorBrush(clr);
            vOnUpdateView();
        }
        public void PDFClose()
        {
            m_view.Content = null;
            m_view.ViewChanged -= vOnViewChanged;
            m_view.SizeChanged -= vOnSizeChanged;
            if (m_items == null) return;
            for (int pgno = 0; pgno < m_items.Length; pgno++)
            {
                UIPageItem item = m_items[pgno];
                if (item != null)
                {
                    m_thread.end_render(item);
                    item.ui_destroy();
                    m_canvas.Children.Remove(item);
                }
            }
            m_canvas = null;
            m_thread.destroy();
            m_thread = null;
            m_items = null;
            m_items_org = null;
        }
        public int[] PDFGetRotate()
        {
            int icnt = m_items_org.Length;
            int[] ret = new int[icnt];
            for(int icur = 0; icur < icnt; icur++)
                ret[icur] = m_items_org[icur].m_rotate;
            return ret;
        }
        public bool[] PDFGetRemove()
        {
            int icnt = m_items_org.Length;
            bool[] ret = new bool[icnt];
            for (int icur = 0; icur < icnt; icur++)
                ret[icur] = m_items_org[icur].m_deleted;
            return ret;
        }
    }
}
