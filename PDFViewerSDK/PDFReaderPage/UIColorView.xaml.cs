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
    public sealed partial class UIColorView : UserControl
    {
        public UIColorView()
        {
            this.InitializeComponent();
            slR.Maximum = 255;
            slG.Maximum = 255;
            slB.Maximum = 255;
        }
        public void init(uint color, bool has_enbale, RoutedEventHandler onOK, RoutedEventHandler onCancel)
        {
            slR.Value = ((color >> 16) & 255);
            slG.Value = ((color >> 8) & 255);
            slB.Value = (color & 255);
            uint alpha = color >> 24;
            btnOK.Click += onOK;
            btnCancel.Click += onCancel;
            if (!has_enbale)
            {
                chkEnable.Visibility = Visibility.Collapsed;
                chkEnable.IsChecked = true;
            }
            else chkEnable.IsChecked = (alpha != 0);
        }
        public uint getColor()
        {
            if(chkEnable.Visibility == Visibility.Visible && !chkEnable.IsChecked.Value) return 0;
            uint r = (uint)slR.Value;
            uint g = (uint)slG.Value;
            uint b = (uint)slB.Value;
            return (0xFF000000 | (r << 16) | (g << 8) | b);
        }

        private void onValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            uint r = (uint)slR.Value;
            uint g = (uint)slG.Value;
            uint b = (uint)slB.Value;
            mPanel.Background = new SolidColorBrush(Color.FromArgb(255, (byte)r, (byte)g, (byte)b));
        }
    }
}
