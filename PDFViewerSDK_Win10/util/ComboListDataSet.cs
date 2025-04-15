using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PDFReader.util
{
    public class ComboItem
    {
        private string _Title;
        private int _Index;

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

        public int Index
        {
            get
            {
                return _Index;
            }
            set
            {
                _Index = value;
            }
        }
    }

    class ComboListDataSet
    {
        private List<ComboItem> _items;

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

        public List<ComboItem> Items
        {
            get
            {
                return _items;
            }
        }

        public void Init(PDFAnnot annot)
        {
            _items = new List<ComboItem>();
            int count = annot.ComboItemCount;
            for (int index = 0; index < count; index++)
            {
                ComboItem item = new ComboItem();
                item.Index = index;
                item.Title = annot.GetComboItem(index);
                _items.Add(item);
            }
        }
    }
}
