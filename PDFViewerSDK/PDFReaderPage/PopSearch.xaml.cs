using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
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
    public sealed partial class PopSearch : UserControl
    {
        public PopSearch()
        {
            this.InitializeComponent();
            mPanel.Background = new SolidColorBrush(Color.FromArgb(255, 255, 255, 160));
            mNext.Tag = 1;
            mPrev.Tag = -1;
        }
        public void init(RoutedEventHandler callback)
        {
            mNext.Click += callback;
            mPrev.Click += callback;
        }
        public bool IsCase()
        {
            return mCase.IsChecked.Value;
        }
        public bool IsWhole()
        {
            return mWord.IsChecked.Value;
        }
        public String getKey()
        {
            return mKey.Text;
        }
    }
}
