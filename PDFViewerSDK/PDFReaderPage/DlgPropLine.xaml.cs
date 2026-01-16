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
    public sealed partial class DlgPropLine : UserControl, IDlgProp
    {
        public DlgPropLine()
        {
            this.InitializeComponent();
        }
        private PDFAnnot m_annot;
        public void loadAnnot(PDFAnnot annot)
        {
            m_annot = annot;
            RelativePanel panel = (RelativePanel)Content;
            btnLStyle.init(panel);
            btnLColor.init(panel);
            btnFColor.init(panel);
            btnLStart.init(panel);
            btnLEnd.init(panel);
            
            txtLWidth.Text = m_annot.StrokeWidth.ToString();
            btnLColor.setColor((uint)m_annot.StrokeColor);
            btnFColor.setColor((uint)m_annot.FillColor);
            btnFColor.setHasEnable(true);

            int lhead = m_annot.LineStyle;
            btnLStart.setHead(lhead & 0xffff);
            btnLEnd.setHead(lhead >> 16);

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
            uint fcolor = btnFColor.getColor();
            if (fcolor == 0)
                m_annot.FillColor = 0;
            else
                m_annot.FillColor = (int)((fcolor & 0xFFFFFF) | (alpha << 24));

            m_annot.LineStyle = btnLStart.getHead() | (btnLEnd.getHead() << 16);

            m_annot.StrokeWidth = float.Parse(txtLWidth.Text);
            m_annot.StrokeDash = btnLStyle.getDash();
        }
    }
}
