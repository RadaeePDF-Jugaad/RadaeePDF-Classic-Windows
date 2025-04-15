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
    public sealed partial class DlgMarkup : UserControl
    {
        public DlgMarkup()
        {
            this.InitializeComponent();
        }
        public int getSelType()
        {
            if (radClipboard.IsChecked.Value) return 100;
            if (radHighlight.IsChecked.Value) return 0;
            if (radUnderline.IsChecked.Value) return 1;
            if (radStrikeout.IsChecked.Value) return 2;
            if (radSquiggly.IsChecked.Value) return 4;
            return -1;
        }
    }
}
