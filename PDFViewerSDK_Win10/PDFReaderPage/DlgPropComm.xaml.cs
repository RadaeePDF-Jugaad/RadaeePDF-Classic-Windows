using RDPDFLib.pdf;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

//https://go.microsoft.com/fwlink/?LinkId=234236 上介绍了“用户控件”项模板

namespace RDPDFReader.annotui
{
    public interface IDlgProp
    {
        void updateAnnot();
    }
    public sealed partial class DlgPropComm : UserControl, IDlgProp
    {
        public DlgPropComm()
        {
            this.InitializeComponent();
        }
        private PDFAnnot m_annot;
        public void loadAnnot(PDFAnnot annot, bool has_fill)
        {
            m_annot = annot;
            RelativePanel panel = (RelativePanel)Content;
            btnLStyle.init(panel);
            btnLColor.init(panel);
            btnFColor.init(panel);

            txtLWidth.Text = m_annot.StrokeWidth.ToString();
            btnLColor.setColor((uint)m_annot.StrokeColor);
            if (has_fill)
            {
                btnFColor.setColor((uint)m_annot.FillColor);
                btnFColor.setHasEnable(true);
            }
            else
            {
                labFColor.Visibility = Visibility.Collapsed;
                btnFColor.Visibility = Visibility.Collapsed;
            }
            slAlpha.Maximum = 255;
            slAlpha.Value = ((m_annot.StrokeColor >> 24) & 255);
            chkLocked.IsChecked = m_annot.Locked;
        }
        public void updateAnnot()
        {
            m_annot.Locked = chkLocked.IsChecked.Value;
            uint alpha = (uint)slAlpha.Value;
            uint lcolor = (btnLColor.getColor() & 0xFFFFFF) | (alpha << 24);
            m_annot.StrokeColor = (int)lcolor;
            if (btnFColor.Visibility == Visibility.Visible)
            {
                uint fcolor = btnFColor.getColor();
                if (fcolor == 0)
                    m_annot.FillColor = 0;
                else
                    m_annot.FillColor = (int)((fcolor & 0xFFFFFF) | (alpha << 24));
            }
            m_annot.StrokeWidth = float.Parse(txtLWidth.Text);
            m_annot.StrokeDash = btnLStyle.getDash();
        }
    }
}
