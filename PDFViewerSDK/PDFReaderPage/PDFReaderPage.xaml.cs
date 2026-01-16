using com.radaee.reader;
using RDPDFLib.pdf;
using RDPDFLib.reader;
using RDPDFLib.view;
using RDPDFReader.annotui;
using System;
using System.Diagnostics;
using System.IO;
using System.Reflection.Metadata.Ecma335;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.Foundation;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Core.Preview;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

// https://go.microsoft.com/fwlink/?LinkId=234238 上介绍了“空白页”项模板

namespace RDPDFReader
{
    public delegate void OnPageCloseHandler(int vmode, PDFPos pos);
    public sealed class PDFReaderPara
    {
        public PDFReaderPara(IRandomAccessStream stream, PDFDoc doc, OnPageCloseHandler close)
        {
            m_doc = doc;
            m_stream = stream;
            m_close = close;
            m_vmode = 0;
            m_pos.pageno = -1;
            m_pos.x = 0;
            m_pos.y = 0;
        }
        ~PDFReaderPara()
        {
            release();
        }
        public void release()
        {
            if (m_doc != null)
            {
                m_doc.Close();
                m_doc = null;
            }
            if (m_stream != null)
            {
                m_stream.Dispose();
                m_stream = null;
            }
            m_close = null;
        }
        public IRandomAccessStream m_stream;
        public PDFDoc m_doc;
        public OnPageCloseHandler m_close;
        public int m_vmode;
        public PDFPos m_pos;
    }
    /// <summary>
    /// 可用于自身或导航至 Frame 内部的空白页。
    /// </summary>
    public sealed partial class PDFReaderPage : Page, IPDFViewListener, IPDFThumbListener
    {
        public static PDFReaderPage sm_inst = null;
        private PDFReaderPara m_pdf;
        private PDFReader m_reader;
        private PDFThumb m_thumb;
        private MenuAnnot m_menu;
        private PopEdit m_pedit;
        private PopCombo m_pcombo;
        public PDFReaderPage()
        {
            this.InitializeComponent();
            //NavigationCacheMode = NavigationCacheMode.Required;
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            NavigationMode mode = e.NavigationMode;
            m_pdf = (PDFReaderPara)e.Parameter;
            m_reader = new PDFReader();
            //m_reader.PDFOpen(mView, m_pdf.m_doc, PDF_LAYOUT_MODE.layout_hdual, this);

            if (m_pdf.m_vmode == 0)
                m_reader.PDFOpen(mView, m_pdf.m_doc, PDF_LAYOUT_MODE.layout_vert, this);
            else
                m_reader.PDFOpen(mView, m_pdf.m_doc, (PDF_LAYOUT_MODE)m_pdf.m_vmode, this);
            m_thumb = new PDFThumb();
            m_thumb.PDFOpen(mThumb, m_pdf.m_doc, this);
            m_menu = new MenuAnnot();
            m_menu.init(onAnnotPerform, onAnnotEdit, onAnnotRemove, onAnnotProperty);

            if (m_pdf.m_pos.pageno >= 0)
                m_reader.PDFGotoPage(m_pdf.m_pos.pageno);
            m_pedit = new PopEdit();
            m_pcombo = new PopCombo();
            Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () => { m_thumb.PDFSetSelPage(m_pdf.m_pos.pageno); });
            enter_normal();
            sm_inst = this;
        }
        private void play_media(String path, PDFAnnot annot)
        {
            RichMediaControl media = new RichMediaControl();
            MediaElement player = media.getPlayer();
            player.Source = new Uri(path);
            player.Play();
            PDFRect rc = annot.Rect;
            float aw = rc.right - rc.left;
            float ah = rc.bottom - rc.top;
            Rect rect = new Rect();
            rect.X = (mRoot.ActualWidth - 400) * 0.5;
            rect.Y = (mRoot.ActualHeight - 400 * ah / aw) * 0.5;
            rect.Width = 400;
            rect.Height = 400 * ah / aw;
            PDFDialog dlg = null;
            dlg = new PDFDialog(/*mRoot,*/ media, /*rect,*/
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                },
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                });
            player.MediaEnded += (object sender1, RoutedEventArgs e1) =>
            {
                player.Stop();
                dlg.Hide();
            };
            media.OnButtonClick += (int btnCode) =>
            {
                switch (btnCode)
                {
                    case 0:
                        //play
                        player.Play();
                        break;
                    case 1:
                        //pause
                        player.Pause();
                        break;
                    case 2:
                        //stop
                        player.Stop();
                        dlg.Hide();
                        break;
                }
            };
            dlg.ShowAsync();
        }
        private void enter_normal()
        {
            mBarCmd.Visibility = Visibility.Visible;
            mThumb.Visibility = Visibility.Visible;
            mBarAnnot.Visibility = Visibility.Collapsed;
            m_annot_status = -1;
            m_menu.dismiss();
            m_pedit.dismiss();
            m_pcombo.dismiss();
            m_annot = null;
            m_annot_pno = -1;
        }
        private void enter_annot()
        {
            mBarCmd.Visibility = Visibility.Collapsed;
            mThumb.Visibility = Visibility.Collapsed;
            mBarAnnot.Visibility = Visibility.Visible;
        }
        private void enter_annot_edit()
        {
            mBarCmd.Visibility = Visibility.Collapsed;
            mThumb.Visibility = Visibility.Collapsed;
            mBarAnnot.Visibility = Visibility.Collapsed;
        }
        private void enter_search()
        {
            mBarCmd.Visibility = Visibility.Collapsed;
            mThumb.Visibility = Visibility.Collapsed;
            mBarAnnot.Visibility = Visibility.Collapsed;
        }
        private void onAnnotPerform(object sender, RoutedEventArgs e)
        {
            m_menu.dismiss();
            m_pedit.dismiss();
            m_pcombo.dismiss();
            m_reader.PDFAnnotPerform();
        }
        private void onAnnotEdit(object sender, RoutedEventArgs e)
        {
            m_menu.dismiss();
            m_pedit.dismiss();
            m_pcombo.dismiss();
            DlgPoptext content = new DlgPoptext();
            content.setLabel(m_annot.PopupLabel);
            content.setPopup(m_annot.PopupText);
            Rect rect = new Rect();
            rect.X = (mRoot.ActualWidth - 280) * 0.5;
            rect.Y = (mRoot.ActualHeight - 210 - 40) * 0.5;
            rect.Width = 280;
            rect.Height = 210;

            PDFDialog dlg = null;
            dlg = new PDFDialog(/*mRoot,*/ content, /*rect,*/
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_annot.PopupLabel = content.getLabel();
                    m_annot.PopupText = content.getPopup();
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                },
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                }
                );
            dlg.ShowAsync();

        }
        private void onAnnotRemove(object sender, RoutedEventArgs e)
        {
            m_menu.dismiss();
            m_pedit.dismiss();
            m_pcombo.dismiss();
            m_reader.PDFAnnotRemove();
            m_reader.PDFModified = true;
        }
        private void onAnnotProperty(object sender, RoutedEventArgs e)
        {
            m_menu.dismiss();
            m_pedit.dismiss();
            m_pcombo.dismiss();
            PDFDialog dlg = null;
            FrameworkElement view = null;
            IDlgProp iprop = null;
            Rect rect;
            int atype = m_annot.Type;
            switch (m_annot.Type)
            {
                case 4:
                case 8:
                    {
                        DlgPropLine prop = new DlgPropLine();
                        prop.loadAnnot(m_annot);
                        view = prop;
                        iprop = prop;
                        rect.X = (mRoot.ActualWidth - 310) * 0.5;
                        rect.Y = (mRoot.ActualHeight - 350 - 40) * 0.5;
                        rect.Width = 310;
                        rect.Height = 350;
                    }
                    break;
                case 3:
                case 5:
                case 6:
                case 7:
                    {
                        DlgPropComm prop = new DlgPropComm();
                        prop.loadAnnot(m_annot, true);
                        view = prop;
                        iprop = prop;
                        rect.X = (mRoot.ActualWidth - 310) * 0.5;
                        rect.Y = (mRoot.ActualHeight - 280 - 40) * 0.5;
                        rect.Width = 310;
                        rect.Height = 280;
                    }
                    break;
                case 15:
                    {
                        DlgPropComm prop = new DlgPropComm();
                        prop.loadAnnot(m_annot, false);
                        view = prop;
                        iprop = prop;
                        rect.X = (mRoot.ActualWidth - 310) * 0.5;
                        rect.Y = (mRoot.ActualHeight - 280 - 40) * 0.5;
                        rect.Width = 310;
                        rect.Height = 280;
                    }
                    break;
                case 9:
                case 10:
                case 11:
                case 12:
                    {
                        DlgPropMarkup prop = new DlgPropMarkup();
                        prop.loadAnnot(m_annot);
                        view = prop;
                        iprop = prop;
                        rect.X = (mRoot.ActualWidth - 310) * 0.5;
                        rect.Y = (mRoot.ActualHeight - 180 - 40) * 0.5;
                        rect.Width = 310;
                        rect.Height = 180;
                    }
                    break;
                case 1:
                case 17:
                    {
                        DlgPropIcon prop = new DlgPropIcon();
                        prop.loadAnnot(m_annot);
                        view = prop;
                        iprop = prop;
                        rect.X = (mRoot.ActualWidth - 310) * 0.5;
                        rect.Y = (mRoot.ActualHeight - 220 - 40) * 0.5;
                        rect.Width = 310;
                        rect.Height = 220;
                    }
                    break;
                default:
                    m_reader.PDFAnnotEnd();
                    return;
                    //break;
            }
            dlg = new PDFDialog(/*mRoot,*/ view, /*rect,*/
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    iprop.updateAnnot();
                    m_reader.PDFUpdateAnnotPage();
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                },
                (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_reader.PDFAnnotEnd();
                    dlg.Hide();
                });
            dlg.ShowAsync();
        }
        public void OnPDFPageUpdated(int pageno)
        {
            if (m_thumb != null)
                m_thumb.PDFUpdatePage(pageno);
        }
        public void OnPDFScaleChanged(double scale)
        {
        }
        public void OnPDFPageChanged(int pageno)
        {
            if (m_thumb != null)
                m_thumb.PDFSetSelPage(pageno);
        }
        public void OnPDFSingleTapped(float x, float y)
        {
            if (mBarCmd.Visibility == Windows.UI.Xaml.Visibility.Visible)
            {
                mBarCmd.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                mThumb.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
            else
            {
                mBarCmd.Visibility = Windows.UI.Xaml.Visibility.Visible;
                mThumb.Visibility = Windows.UI.Xaml.Visibility.Visible;
            }
        }
        public void OnPDFPageTapped(int pageno)
        {
        }
        public void OnPDFLongPressed(float x, float y)
        {
        }
        public void OnPDFFound(bool found)
        {
        }
        public void OnPDFSelecting(Canvas canvas, PDFRect rect1, PDFRect rect2)
        {
        }
        public void OnPDFSelected()
        {
            DlgMarkup content = new DlgMarkup();
            Rect rect;
            rect.X = (mRoot.ActualWidth - 180) * 0.5;
            rect.Y = (mRoot.ActualHeight - 210 - 40) * 0.5;
            rect.Width = 180;
            rect.Height = 210;
            PDFDialog dlg = null;
            dlg = new PDFDialog(/*mRoot,*/ content, /*rect,*/
                (ContentDialog sender, ContentDialogButtonClickEventArgs e) =>
                {
                    dlg.Hide();
                    int itype = content.getSelType();
                    if (itype < 0) return;
                    if (itype == 100)
                    {
                        Clipboard.Clear();
                        DataPackage dp = new DataPackage();
                        dp.SetData(StandardDataFormats.Text, m_reader.PDFSelGetText());
                        Clipboard.SetContent(dp);
                    }
                    else
                    {
                        uint color = 0xFFFFFF00;
                        if (itype == 1)
                            color = 0xFF0000C0;
                        else if (itype == 2)
                            color = 0xFFC00000;
                        else if (itype == 4)
                            color = 0xFF00C000;
                        m_reader.PDFSelSetMarkup(color, itype);
                    }
                    m_reader.PDFSelEnd();
                },
                (ContentDialog sender, ContentDialogButtonClickEventArgs e) =>
                {
                    dlg.Hide();
                    m_reader.PDFSelEnd();
                });
            dlg.ShowAsync();
        }
        private PDFAnnot m_annot;
        private PDFRect m_annot_rect;
        private int m_annot_pno;
        public void OnPDFAnnotClicked(PDFPage page, int pageno, PDFAnnot annot, PDFRect rect)
        {
            m_annot = annot;
            m_annot_pno = pageno;
            m_annot_rect = rect;
            if (m_pdf.m_doc.CanSave && annot.EditType > 0)//edit box
            {
                Rect rc;
                rc.X = rect.left;
                rc.Y = rect.top;
                rc.Width = rect.right - rect.left;
                rc.Height = rect.bottom - rect.top;

                double windowWidth = Window.Current.Bounds.Width;
                if (rc.X + rc.Width > windowWidth)
                {
                    rc.X = windowWidth - rc.Width;
                }

                Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
                {
                    m_pedit.Text = annot.EditText;
                    m_pedit.show(mView, rc, annot.EditTextSize * m_reader.PDFGetScale(), annot.EditType == 3);
                    enter_annot_edit();
                });
            }
            else if (m_pdf.m_doc.CanSave && annot.ComboItemCount > 0)
            {
                Rect rc;
                rc.X = rect.left;
                rc.Y = rect.top;
                rc.Width = rect.right - rect.left;
                rc.Height = rect.bottom - rect.top;

                double windowWidth = Window.Current.Bounds.Width;
                if (rc.X + rc.Width > windowWidth)
                {
                    rc.X = windowWidth - rc.Width;
                }

                int cnt = annot.ComboItemCount;
                String[] opts = new string[cnt];
                for (int cur = 0; cur < cnt; cur++)
                    opts[cur] = annot.GetComboItem(cur);
                m_pcombo.show(mView, rc, m_reader.PDFGetScale(), opts, (object sender, RoutedEventArgs e) =>
                {
                    Button ele = (Button)sender;
                    int idx = (int)ele.Tag;
                    m_annot.ComboItemSel = idx;
                    m_pcombo.dismiss();
                    m_reader.PDFUpdateAnnotPage();
                    m_reader.PDFAnnotEnd();
                });
                enter_annot_edit();
            }
            else
            {
                Point pt;
                pt.X = rect.left;
                pt.Y = rect.top - 40;
                if (pt.Y < 0)
                    pt.Y = rect.bottom;
                m_menu.show(mView, annot.Type, pt);
                enter_annot_edit();
            }
        }
        public void OnPDFAnnotEnd()
        {
            if (m_pedit.isShow())
            {
                m_annot.EditText = m_pedit.Text;
                m_reader.PDFUpdateAnnotPage();
            }
            enter_normal();
        }
        public void OnPDFAnnotGoto(int pageno)
        {
            m_reader.PDFGotoPage(pageno);
        }
        public void OnPDFAnnotURI(string uri)
        {
        }
        private string Between(string STR, string FirstString, string LastString)
        {
            string FinalString;
            int Pos1 = STR.IndexOf(FirstString) + FirstString.Length;
            int Pos2 = STR.IndexOf(LastString);
            FinalString = STR.Substring(Pos1, Pos2 - Pos1);
            return FinalString;
        }
        private String m_media;
        public void OnPDFAnnotRichMedia(PDFAnnot annot)
        {
            try
            {
                if (annot != null)
                {
                    String movie;
                    if (annot.RichMediaItemCount > 0)
                    {
                        movie = Between(annot.GetRichMediaItemPara(0), "source=", "&");
                    }
                    else
                        movie = annot.GetMovieName();
                    if (movie != "")
                    {
                        movie = movie.Replace("%20", " "); //to solve the problem of the library returning the space replaced with %20
                        StorageFolder folder = ApplicationData.Current.TemporaryFolder;
                        String path = folder.Path;
                        m_media = path + "\\" + movie;
                        if (annot.GetRichMediaData(movie, m_media))
                            play_media(m_media, annot);
                    }
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
            }
        }
        public void OnPDFAnnotRendition(PDFAnnot annot)
        {
        }
        public void OnPDFAnnotPopup(PDFAnnot annot, string subj, string text)
        {
        }
        public void OnPDFAnnotRemoteDest(string dest)
        {
        }
        public void OnPDFAnnotFileLink(string filelink)
        {
        }

        private void vClose(bool close_doc)
        {
            if (m_reader == null) return;
            m_thumb.PDFClose();
            m_thumb = null;

            m_pdf.m_close?.Invoke((int)m_reader.PDFViewMode, m_reader.PDFGetCurPos());
            m_reader.PDFClose();
            m_reader = null;

            m_menu.uninit(onAnnotPerform, onAnnotEdit, onAnnotRemove, onAnnotProperty);
            m_menu = null;
            m_pedit = null;
            m_pcombo = null;
            mAnnot.Click -= mAnnot_Click;
            mBack.Click -= mBack_Click;
            mCancel.Click -= mCancel_Click;
            mOk.Click -= mOk_Click;
            mTool.Click -= mTool_Click;
            mVMode.Click -= mVMode_Click;
            /*
            mAnnot = null;
            mBack = null;
            mCancel = null;
            mOk = null;
            mTool = null;
            mVMode = null;

            mBarAnnot = null;
            mBarCmd = null;
            mView = null;
            mThumb = null;
            mRoot = null;
            */
            if (close_doc) m_pdf.release();
            m_pdf = null;

            sm_inst = null;
            if (m_def != null)
                m_def.Complete();
            else
                Frame.GoBack();
        }
        private Deferral m_def;
        public bool IsModified()
        {
            return (m_reader.PDFModified && m_pdf.m_doc.CanSave);
        }
        public void OnAppClose(SystemNavigationCloseRequestedPreviewEventArgs eve)
        {
            if (m_reader.PDFModified && m_pdf.m_doc.CanSave)
            {
                ContentDialog dlg = new ContentDialog
                {
                    Title = "Save file?",
                    Content = "Do you want save PDF file?",
                    PrimaryButtonText = "Yes",
                    SecondaryButtonText = "No",
                    CloseButtonText = "Cancel"
                };
                m_def = eve.GetDeferral();
                dlg.DefaultButton = ContentDialogButton.Primary;
                dlg.PrimaryButtonClick += (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_pdf.m_doc.Save();
                    vClose(true);
                };
                dlg.SecondaryButtonClick += (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    vClose(true);
                };
                dlg.CloseButtonClick += (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    eve.Handled = true;
                    m_def.Complete();
                };
                dlg.ShowAsync();
            }
            else
            {
                m_def = eve.GetDeferral();
                vClose(true);
            }
        }
        private void mBack_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            if (m_reader == null)
            {
                m_pdf.m_stream.Dispose();
                Frame.GoBack();
            }
            else if (m_reader.PDFModified && m_pdf.m_doc.CanSave)
            {
                ContentDialog dlg = new ContentDialog
                {
                    Title = "Save file?",
                    Content = "Do you want save PDF file?",
                    PrimaryButtonText = "Yes",
                    SecondaryButtonText = "No",
                    CloseButtonText = "Cancel"
                };
                m_def = null;
                dlg.DefaultButton = ContentDialogButton.Primary;
                dlg.PrimaryButtonClick += (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    m_pdf.m_doc.Save();
                    vClose(true);
                };
                dlg.SecondaryButtonClick += (ContentDialog sender1, ContentDialogButtonClickEventArgs e1) =>
                {
                    vClose(true);
                };
                dlg.ShowAsync();
            }
            else
                vClose(true);
        }

        public void OnPDFPageSelected(int pageno)
        {
            m_reader.PDFGotoPage(pageno);
        }
        private void mVMode_Click(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            int item_height = 40;
            int item_width = 230;
            int item_cnt = 6;
            int menu_x = 100;
            int menu_y = 78;
            StackPanel panel = new StackPanel();
            panel.Orientation = Orientation.Vertical;
            PDFPopup pop = null;
            panel.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            panel.IsTapEnabled = true;
            panel.Tapped += (object obj, TappedRoutedEventArgs eve) =>
            {
                Point pos = eve.GetPosition(panel);
                switch ((int)(pos.Y / item_height))
                {
                    case 1:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_horz;
                        break;
                    case 2:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_dual;
                        break;
                    case 3:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_dual_cover;
                        break;
                    case 4:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_hdual;
                        break;
                    case 5:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_hsingle;
                        break;
                    default:
                        m_reader.PDFViewMode = PDF_LAYOUT_MODE.layout_vert;
                        break;
                }
                pop.dismiss();
            };

            Image img;
            TextBlock text;
            StackPanel item;

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_view_vert.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Vertical";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_view_horz.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Horizon";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_view_horz_cover.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Dual Page With Cover";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_view_dual_page.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Dual Page";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_horz_dual_page.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Horizontal Dual Page";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_single_page.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Single Page";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            Point pt = mVMode.RenderTransformOrigin;
            Rect rect = new Rect(menu_x, menu_y, item_width, item_height * item_cnt);
            pop = new PDFPopup(mRoot, panel, rect);
        }
        private void OnPageConfirmed(int[] rotates, bool[] deletes)
        {
            if (rotates == null || deletes == null) return;
            m_reader.PDFSaveView();
            m_thumb.PDFSaveView();
            PDFDoc doc = m_pdf.m_doc;
            int cur = deletes.Length;
            bool modified = false;
            while (cur > 0)
            {
                cur--;
                if (deletes[cur])
                {
                    doc.RemovePage(cur);
                    modified = true;
                }
                else
                {
                    int rot_org = rotates[cur] >> 16;
                    int rot_new = rotates[cur] & 0xFFFF;
                    if (rot_org != rot_new)
                    {
                        //if (rot_new > rot_org) rot_new -= rot_org;
                        //else rot_new = rot_new + 360 - rot_org;
                        doc.SetPageRotate(cur, rot_new);
                        modified = true;
                    }
                }
            }
            m_thumb.PDFRestoreView();
            m_reader.PDFRestoreView();
            if (modified) m_reader.PDFModified = true;
        }
        private void mTool_Click(object sender, RoutedEventArgs e)
        {
            int item_height = 40;
            int item_width = 160;
            int item_cnt = 7;
            int menu_x = 180;
            int menu_y = 78;
            StackPanel panel = new StackPanel();
            panel.Orientation = Orientation.Vertical;
            PDFPopup pop = null;
            panel.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            panel.IsTapEnabled = true;
            panel.Tapped += (object obj, TappedRoutedEventArgs eve) =>
            {
                Point pos = eve.GetPosition(panel);
                switch ((int)(pos.Y / item_height))
                {
                    case 0://undo
                        m_reader.PDFUndo();
                        break;
                    case 1://redo
                        m_reader.PDFRedo();
                        break;
                    case 2://search
                        {
                            enter_search();
                            PopSearch search = new PopSearch();
                            Rect rcl;
                            rcl.X = 0;
                            rcl.Y = 0;
                            rcl.Width = 220;
                            rcl.Height = 150;
                            String skey = "";
                            bool mcase = false;
                            bool mwhole = false;
                            PDFPopup pop_out = new PDFPopup(mRoot, search, rcl);
                            search.init((object sender1, RoutedEventArgs e1) =>
                            {
                                String sskey = search.getKey();
                                bool mmcase = search.IsCase();
                                bool mmwhole = search.IsWhole();
                                Button btn = (Button)sender1;
                                if (skey != sskey || mcase != mmcase || mwhole != mmwhole)
                                {
                                    skey = sskey;
                                    mcase = mmcase;
                                    mwhole = mmwhole;
                                    m_reader.PDFFindStart(skey, mcase, mwhole);
                                }
                                m_reader.PDFFind((int)btn.Tag);
                            });
                        }
                        break;
                    case 3://selection
                        m_reader.PDFSelStart();
                        break;
                    case 4://meta
                        {
                            DlgMeta dlg = new DlgMeta();
                            dlg.loadMeta(m_pdf.m_doc);
                            dlg.ShowAsync();
                        }
                        break;
                    case 5://outline
                        {
                            PopOutline outline = new PopOutline();
                            Rect rcl;
                            rcl.X = 0;
                            rcl.Y = mBarCmd.Height;
                            rcl.Width = 200;
                            rcl.Height = mRoot.Height - rcl.Y;
                            PDFPopup pop_out = new PDFPopup(mRoot, outline, rcl);

                            outline.loadOutline(m_pdf.m_doc, (object sender1, RoutedEventArgs e1) =>
                            {
                                Button btn = (Button)sender1;
                                PDFOutline iout = (PDFOutline)btn.Tag;
                                if (iout.dest >= 0) OnPDFAnnotGoto(iout.dest);
                                pop_out.dismiss();
                            });
                        }
                        break;
                    case 6://page editing
                        if (!m_pdf.m_doc.CanSave)
                        {
                            pop.dismiss();
                            Windows.UI.Popups.MessageDialog msgDlg = new Windows.UI.Popups.MessageDialog("The PDF File is not writable, maybe opened by other App.") { Title = "Readonly" };
                            msgDlg.Commands.Add(new Windows.UI.Popups.UICommand("OK", uiCommand => { }));
                            msgDlg.ShowAsync();
                            return;
                        }
                        mPages.PDFOpen(m_pdf.m_doc, OnPageConfirmed);
                        break;
                    default:
                        break;
                }
                pop.dismiss();
            };

            Image img;
            TextBlock text;
            StackPanel item;

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_undo.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Undo";
            text.VerticalAlignment = VerticalAlignment.Bottom;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_redo.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Redo";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/view_search.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Search";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/annot_text.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Text Selection";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Width = item_height;
            img.Height = item_height;
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_meta.png", UriKind.Absolute));
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Meta";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_menu.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Outline";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_manage_pages.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Page Editing";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            Point pt = mVMode.RenderTransformOrigin;
            Rect rect = new Rect(menu_x, menu_y, item_width, item_height * item_cnt);
            pop = new PDFPopup(mRoot, panel, rect);
        }
        private int m_annot_status = -1;
        private void mAnnot_Click(object sender, RoutedEventArgs e)
        {
            if (!m_pdf.m_doc.CanSave)
            {
                Windows.UI.Popups.MessageDialog msgDlg = new Windows.UI.Popups.MessageDialog("The PDF File is not writable, maybe opened by other App.") { Title = "Readonly" };
                msgDlg.Commands.Add(new Windows.UI.Popups.UICommand("OK", uiCommand => { }));
                msgDlg.ShowAsync();
                return;
            }
            int item_height = 40;
            int item_width = 160;
            int item_cnt = 9;
            int menu_x = 260;
            int menu_y = 78;
            StackPanel panel = new StackPanel();
            panel.Orientation = Orientation.Vertical;
            PDFPopup pop = null;
            panel.Background = new SolidColorBrush(Color.FromArgb(255, 224, 224, 224));
            panel.IsTapEnabled = true;
            panel.Tapped += (object obj, TappedRoutedEventArgs eve) =>
            {
                Point pos = eve.GetPosition(panel);
                m_annot_status = (int)(pos.Y / item_height);
                switch (m_annot_status)
                {
                    case 0:
                        m_reader.PDFInkStart();
                        break;
                    case 1:
                        m_reader.PDFNoteStart();
                        break;
                    case 2:
                        m_reader.PDFLineStart();
                        break;
                    case 3:
                        m_reader.PDFRectStart();
                        break;
                    case 4:
                        m_reader.PDFEllipseStart();
                        break;
                    case 5:
                        m_reader.PDFStampStart();
                        break;
                    case 6:
                        m_reader.PDFEditTextStart();
                        break;
                    case 7:
                        m_reader.PDFPolygonStart();
                        break;
                    case 8:
                        m_reader.PDFPolylineStart();
                        break;
                    default:
                        break;
                }
                pop.dismiss();
                enter_annot();
            };

            Image img;
            TextBlock text;
            StackPanel item;

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/annot_line.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Ink";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/annot_rect_text.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Note";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_select.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Line";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/annot_rect.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Rectangle";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/annot_ellipse.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Ellipse";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_stamp.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Stamp";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_edit_box.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Editbox";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_polygon.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Polygon";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            item = new StackPanel();
            item.Orientation = Orientation.Horizontal;
            img = new Image();
            img.Source = new BitmapImage(new Uri("ms-appx:///Assets/imgs/icon_polyline.png", UriKind.Absolute));
            img.Width = item_height;
            img.Height = item_height;
            item.Children.Add(img);
            text = new TextBlock();
            text.Text = "Polyline";
            text.VerticalAlignment = VerticalAlignment.Center;
            text.Width = item_width - item_height;
            text.Height = item_height;
            item.Children.Add(text);
            panel.Children.Add(item);

            Point pt = mVMode.RenderTransformOrigin;
            Rect rect = new Rect(menu_x, menu_y, item_width, item_height * item_cnt);
            pop = new PDFPopup(mRoot, panel, rect);
        }

        private void mCancel_Click(object sender, RoutedEventArgs e)
        {
            switch (m_annot_status)
            {
                case 0:
                    m_reader.PDFInkCancel();
                    break;
                case 1:
                    m_reader.PDFNoteCancel();
                    break;
                case 2:
                    m_reader.PDFLineCancel();
                    break;
                case 3:
                    m_reader.PDFRectCancel();
                    break;
                case 4:
                    m_reader.PDFEllipseCancel();
                    break;
                case 5:
                    m_reader.PDFStampCancel();
                    break;
                case 6:
                    m_reader.PDFEditTextCancel();
                    break;
                case 7:
                    m_reader.PDFPolygonCancel();
                    break;
                case 8:
                    m_reader.PDFPolylineCancel();
                    break;
                default:
                    break;
            }
            enter_normal();
            m_annot_status = -1;
        }

        private void mOk_Click(object sender, RoutedEventArgs e)
        {
            switch (m_annot_status)
            {
                case 0:
                    m_reader.PDFInkEnd();
                    break;
                case 1:
                    m_reader.PDFNoteEnd();
                    break;
                case 2:
                    m_reader.PDFLineEnd();
                    break;
                case 3:
                    m_reader.PDFRectEnd();
                    break;
                case 4:
                    m_reader.PDFEllipseEnd();
                    break;
                case 5:
                    m_reader.PDFStampEnd();
                    break;
                case 6:
                    m_reader.PDFEditTextEnd();
                    break;
                case 7:
                    m_reader.PDFPolygonEnd();
                    break;
                case 8:
                    m_reader.PDFPolylineEnd();
                    break;
                default:
                    break;
            }
            enter_normal();
            m_annot_status = -1;
        }
    }
}
