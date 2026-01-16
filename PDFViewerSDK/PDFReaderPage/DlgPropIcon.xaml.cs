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
    public sealed partial class DlgPropIcon : UserControl, IDlgProp
    {
        public DlgPropIcon()
        {
            this.InitializeComponent();
        }
        private PDFAnnot m_annot;
        public void loadAnnot(PDFAnnot annot)
        {
            m_annot = annot;
            RelativePanel panel = (RelativePanel)Content;
            btnFColor.init(panel);
            btnFColor.setHasEnable(false);
            btnIcon.init(panel, annot.Type, annot.Icon);

            uint color = (uint)m_annot.FillColor;
            btnFColor.setColor(color);

            slAlpha.Maximum = 255;
            slAlpha.Value = ((color >> 24) & 255);
            chkLocked.IsChecked = m_annot.Locked;
        }
        public void updateAnnot()
        {
            m_annot.Locked = chkLocked.IsChecked.Value;
            uint alpha = (uint)slAlpha.Value;
            uint color = btnFColor.getColor() & 0xffffff;
            m_annot.FillColor = (int)((alpha << 24) | color);
            m_annot.Icon = btnIcon.getIcon();
        }
    }
}
