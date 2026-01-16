using RDPDFLib.pdf;
using System;
using System.Collections.Generic;

namespace PDFViewerSDK_Win10
{
    public class MenuItem
    {
        private string _Title;
        private int _Page;

        public string Title
        {
            get
            {
                return _Title;
            }
            set
            {
                _Title = value;
            }
        }

        public int Page
        {
            get
            {
                return _Page;
            }
            set
            {
                _Page = value;
            }
        }
    }

    public class MenuDataSet
    {
        private List<MenuItem> _items;
        private int mLevel;
        private PDFDoc mDoc;

        public int Length
        {
            get
            {
                if (_items != null)
                {
                    return _items.Count;
                }
                else
                    return 0;
            }
        }

        public List<MenuItem> Items
        {
            get
            {
                return _items;
            }
        }

        public void Init(PDFDoc doc)
        {
            mDoc = doc;
            mLevel = 0;
            _items = new List<MenuItem>();
            getList(null);
        }

        private void getList(PDFOutline node)
        {
            int level = mLevel;
            PDFOutline current;
            if (node != null)
            {
                current = node;
            }
            else
            {
                current = mDoc.GetRootOutline();
            }
            if (current == null)
                return;
            else
            {
                MenuItem item = new MenuItem();
                item.Page = current.dest;
                String title = current.label;
                title = "|" + title;
                for (int i = 0; i < level; i++)
                    title = "    " + title;
                item.Title = title;
                _items.Add(item);
                PDFOutline child;
                if ((child = current.GetChild()) != null)
                {
                    getList(child);
                }
            }

            while ((current = current.GetNext()) != null)
            {

                MenuItem item = new MenuItem();
                item.Page = current.dest;
                String title = current.label;
                title = "|" + title;
                for (int i = 0; i < level; i++)
                    title = "        " + title;
                item.Title = title;
                _items.Add(item);
                PDFOutline child;
                if ((child = current.GetChild()) != null)
                {
                    mLevel++;
                    getList(child);
                }
            }
            mLevel--;
        }
    }
}
