using PDFViewerSDK_Win10.AnnotControls;
using PDFViewerSDK_Win10.OptionPanelControls;
using PDFViewerSDK_Win10.SettingsControls;
using PDFViewerSDK_Win10.view;
using RDPDFLib.pdf;
using System;
using System.Diagnostics;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.DataTransfer;
using Windows.Foundation;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.System;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Notifications;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace PDFViewerSDK_Win10
{
    public delegate void OnPageCloseHandler(PDFView.PDFPos pos);

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class PDFReaderPage : Page, PDFView.PDFViewListener
    {
        IRandomAccessStream m_stream;
        private PDFDoc m_doc;
        private PDFView m_view;
        private AnnotControl mAnnotControl;
        private SearchControl mSearchPanel;
        private EditAnnotControl mEditAnnotControl;
        private RichMediaControl mRichmediaControl;
        private AnnotControl.AnnotType mAnnotType = AnnotControl.AnnotType.TypeNone;
        private PasswordControl mPasswordControl = null;
        private SaveDocControl mSaveDocControl = null;
        private FormEditControl mFormEditControl = null;
        private FormComboControl mFormComboControl = null;
        private string mSearchKey = "";
        private bool mIsModified;
        private MediaElement mMediaElement;

        private MenuDataSet mMenuData;

        private bool mIsDualPageMode = false;

        //private bool isPDFZooming = false;
        private TextSelectionControl mTextSelectionDialog;
        private PDFAnnot mAnnot;
        private TextAnnotControl mTextAnnotDialog = null;

        static public event OnPageCloseHandler OnPageClose;

        public PDFReaderPage()
        {
            this.InitializeComponent();
            CoreWindow rcWindow = Window.Current.CoreWindow;
            Rect rcScreen = rcWindow.Bounds;
            mPDFView.Width = rcScreen.Width;
            mPDFView.Height = rcScreen.Height;
            Window.Current.CoreWindow.Dispatcher.AcceleratorKeyActivated += AcceleratorKeyActivated;
        }

        private void AcceleratorKeyActivated(CoreDispatcher sender, AcceleratorKeyEventArgs args)
        {
            if (m_view == null)
                return;
            if (mAnnotType != AnnotControl.AnnotType.TypeNone)
                return;
            if (mFormEditControl != null && mFormEditControl.isShowing())
                return;

            if (args.EventType.ToString().Equals("KeyDown"))
            {
                switch (args.VirtualKey)
                {
                    case VirtualKey.Right:
                    case VirtualKey.Space:
                        {
                            //PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                            PDFVPage vPage = m_view.getCurrentVPage();
                            int pageNo = vPage.GetPageNo();
                            if (pageNo < m_doc.PageCount - 1)
                            {
                                if (mIsDualPageMode)
                                    pageNo += 2;
                                else
                                    pageNo++;
                                m_view.vGotoPage(pageNo);
                            }
                            args.Handled = true;
                            break;
                        }
                    case VirtualKey.Left:
                        {
                            //PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                            PDFVPage vPage = m_view.getCurrentVPage();
                            int pageNo = vPage.GetPageNo();
                            if (pageNo > 0)
                            {
                                if (mIsDualPageMode)
                                    pageNo -= 2;
                                else
                                    pageNo--;
                                m_view.vGotoPage(pageNo);
                            }
                        }
                        break;
                    case VirtualKey.Up:
                        {
                            PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                            if (pos.pageno > 0)
                            {
                                pos.pageno = 0;
                                m_view.vSetPos(pos, 0, 0);
                            }
                        }
                        break;
                    case VirtualKey.Down:
                        {
                            PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                            if (pos.pageno < m_doc.PageCount - 1)
                            {
                                pos.pageno = m_doc.PageCount - 1;
                                m_view.vSetPos(pos, 0, 0);
                            }
                        }
                        break;
                    case VirtualKey.F:
                        {
                            var ctrl = Window.Current.CoreWindow.GetKeyState(VirtualKey.Control);
                            if (ctrl.HasFlag(CoreVirtualKeyStates.Down))
                            {
                                PDFOptionPanel.Children.Clear();
                                PDFOptionPanel.Children.Add(mSearchPanel);
                                mAppBar.IsOpen = true;
                                mSearchPanel.setFocus();
                            }
                        }
                        break;
                    case VirtualKey.F3:
                    case VirtualKey.Enter:
                        {
                            string searchKey = mSearchPanel.getKey();
                            if (searchKey.Length == 0)
                            {
                                mSearchKey = "";
                                mSearchPanel.enableUI(true);
                                m_view.vFindEnd();
                                break;
                            }
                            if (!searchKey.Equals(mSearchKey))
                            {
                                mSearchKey = searchKey;
                                bool matchCase = mSearchPanel.getMatchCase();
                                bool matchWholeWord = mSearchPanel.getMatchWholeWord();
                                mSearchPanel.enableUI(false);
                                m_view.vFindStart(mSearchKey, matchCase, matchWholeWord);
                            }

                            var shift = Window.Current.CoreWindow.GetKeyState(VirtualKey.Shift);
                            if (shift.HasFlag(CoreVirtualKeyStates.Down))
                            {
                                m_view.vFind(-1);
                            }
                            else
                            {
                                m_view.vFind(1);
                            }
                        }
                        break;
                }

            }
        }

        private async void exportPDFAsync(StorageFile source)
        {
            StorageFolder storageFolder = ApplicationData.Current.LocalFolder;
            StorageFile copiedFile = await source.CopyAsync(storageFolder, "tmp.pdf", NameCollisionOption.ReplaceExisting);
            IRandomAccessStream tmpStream = await copiedFile.OpenAsync(FileAccessMode.ReadWrite);
            PDFDoc srcDoc = new PDFDoc();
            PDF_ERROR err = srcDoc.Open(tmpStream, "");
            if (err == PDF_ERROR.err_ok)
            {
                int pageCount = srcDoc.PageCount;
                for (int i = 0; i < pageCount; i++)
                {
                    PDFPage page = srcDoc.GetPage(i);
                    page.ObjsStart();
                    page.FlatAnnots();
                    srcDoc.Save();
                    page.Close();
                }

                for (int i = 0; i < pageCount; i++)
                {
                    StorageFile newFile = await storageFolder.CreateFileAsync((i + 1) + ".pdf", CreationCollisionOption.ReplaceExisting);
                    IRandomAccessStream stream = await newFile.OpenAsync(FileAccessMode.ReadWrite);
                    PDFDoc newDoc = new PDFDoc();
                    if (newDoc.Create(stream) == PDF_ERROR.err_ok)
                    {
                        PDFImportCtx ctx = newDoc.ImportStart(srcDoc);
                        newDoc.ImportPage(ctx, i, 0);
                        newDoc.Save();
                        newDoc.Close();
                    }
                    stream.Dispose();
                }
            }
        }


        private void OnViewModeDialogClose(int mode)
        {
            PDFView.PDFPos pos = m_view.vGetPos(0, 0);
            switch (mode)
            {
                case 0:
                    //Vert
                    m_view.vClose();
                    m_view = new PDFViewVert(mPDFView);
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
                    m_view.vSetPos(pos, 0, 0);
                    mIsDualPageMode = false;
                    break;
                case 1:
                    //Horz
                    m_view.vClose();
                    m_view = new PDFViewHorz(mPDFView);
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
                    m_view.vSetPos(pos, 0, 0);
                    mIsDualPageMode = false;
                    break;
                case 2:
                    //Dual Page
                    m_view.vClose();
                    m_view = new PDFViewDual(mPDFView);
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
                    m_view.vSetPos(pos, 0, 0);
                    int page = Convert.ToInt32(mPageInput.Text);
                    m_view.vGotoPage(--page);
                    mIsDualPageMode = true;
                    break;
                case 3:
                    //Single Page
                    m_view.vClose();
                    m_view = new PDFViewDual(mPDFView);
                    bool[] paras = new bool[m_doc.PageCount];
                    int cur = 0;
                    while (cur < paras.Length)
                    {
                        paras[cur] = false;
                        cur++;
                    }
                    ((PDFViewDual)m_view).vSetLayoutPara(null, paras, false);
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
                    m_view.vSetPos(pos, 0, 0);
                    page = Convert.ToInt32(mPageInput.Text);
                    m_view.vGotoPage(--page);
                    mIsDualPageMode = false;
                    break;
            }
        }

        /// <summary>
        /// 在此页将要在 Frame 中显示时进行调用。
        /// </summary>
        /// <param name="e">描述如何访问此页的事件数据。Parameter
        /// 属性通常用于配置页。</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            m_stream = (IRandomAccessStream)e.Parameter;
            m_doc = new PDFDoc();
            PDF_ERROR err = m_doc.Open(m_stream, "");
            switch (err)
            {
                case PDF_ERROR.err_ok:
                    //PDFView.init();                   

                    if (ApplicationData.Current.LocalSettings.Values.ContainsKey("View_mode"))
                        PDFView.viewMode = Convert.ToInt32(ApplicationData.Current.LocalSettings.Values["View_mode"]);
                    PDFView.viewMode = 3;
                    switch (PDFView.viewMode)
                    {
                        case 0:
                            m_view = new PDFViewVert(mPDFView);
                            mIsDualPageMode = false;
                            break;
                        case 1:
                            m_view = new PDFViewHorz(mPDFView);
                            mIsDualPageMode = false;
                            break;
                        case 2:
                            m_view = new PDFViewDual(mPDFView);
                            mIsDualPageMode = true;
                            break;
                        case 3:
                            m_view = new PDFViewDual(mPDFView);
                            bool[] paras = new bool[m_doc.PageCount];
                            int cur = 0;
                            while (cur < paras.Length)
                            {
                                paras[cur] = false;
                                cur++;
                            }
                            ((PDFViewDual)m_view).vSetLayoutPara(null, paras, false);
                            mIsDualPageMode = false;
                            break;
                        default:
                            m_view = new PDFViewVert(mPDFView);
                            mIsDualPageMode = false;
                            break;
                    }
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);

                    initOptionPanels();
                    mAnnotType = AnnotControl.AnnotType.TypeNone;
                    mIsModified = false;
                    mMenuData = new MenuDataSet();
                    mMenuData.Init(m_doc);
                    viewMenuBtn.IsEnabled = (mMenuData.Length > 0);
                    mPageDisplay.Text = "/" + m_doc.PageCount;
                    GotoHistoryPage();
                    break;
                case PDF_ERROR.err_password:
                    mPasswordControl = new PasswordControl();
                    mPasswordControl.OnDialogClose += OnPasswordDialogClose;
                    mPasswordControl.Show();
                    break;
                default:
                    break;
            }
            //m_view.vSelStart();
            //m_view.vNoteStart();
            //m_view.vInkStart();
            //m_view.vRectStart();
            //m_view.vLineStart();
        }

        private static string MD5Cryptstring(MD5 md5Hash, string input)
        {
            byte[] data = md5Hash.ComputeHash(Encoding.Default.GetBytes(input));

            StringBuilder sBuilder = new StringBuilder();

            for (int i = 0; i < data.Length; i++)
            {
                sBuilder.Append(data[i].ToString("x2"));
            }

            return sBuilder.ToString();
        }

        private void OnPasswordDialogClose(string password)
        {
            if (password.Length == 0)
            {
                //Cancel
                mPasswordControl.Dismiss();
            }
            PDF_ERROR err = m_doc.Open(m_stream, password);
            switch (err)
            {
                case PDF_ERROR.err_ok:
                    //PDFView.init();
                    if (ApplicationData.Current.LocalSettings.Values.ContainsKey("View_mode"))
                        PDFView.viewMode = Convert.ToInt32(ApplicationData.Current.LocalSettings.Values["View_mode"]);
                    switch (PDFView.viewMode)
                    {
                        case 0:
                            m_view = new PDFViewVert(mPDFView);
                            mIsDualPageMode = false;
                            break;
                        case 1:
                            m_view = new PDFViewHorz(mPDFView);
                            mIsDualPageMode = false;
                            break;
                        case 2:
                            m_view = new PDFViewDual(mPDFView);
                            mIsDualPageMode = true;
                            break;
                        default:
                            m_view = new PDFViewVert(mPDFView);
                            mIsDualPageMode = false;
                            break;
                    }
                    m_view.vOpen(m_doc, 4, 0xFFCCCCCC, this);
                    initOptionPanels();
                    mAnnotType = AnnotControl.AnnotType.TypeNone;
                    mIsModified = false;
                    mPasswordControl.Dismiss();
                    mMenuData = new MenuDataSet();
                    mMenuData.Init(m_doc);
                    viewMenuBtn.IsEnabled = (mMenuData.Length > 0);
                    mPageDisplay.Text = "/" + m_doc.PageCount;
                    GotoHistoryPage();
                    break;
                case PDF_ERROR.err_password:
                    mPasswordControl.showHint();
                    break;
                default:
                    mPasswordControl.Dismiss();
                    break;
            }
        }

        private void GotoHistoryPage()
        {
            int page;
            int x;
            int y;
            if (MainPage.FileToken.Equals(string.Empty))
            {
                mPageInput.Text = "1";
                return;
            }

            if (ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_page"))
            {
                page = Convert.ToInt32(ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_page"]);
            }
            else
            {
                mPageInput.Text = "1";
                return;
            }

            if (ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_x"))
            {
                x = Convert.ToInt32(ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_x"]);
            }
            else
            {
                mPageInput.Text = "1";
                return;
            }

            if (ApplicationData.Current.LocalSettings.Values.ContainsKey(MainPage.FileToken + "_y"))
            {
                y = Convert.ToInt32(ApplicationData.Current.LocalSettings.Values[MainPage.FileToken + "_y"]);
            }
            else
            {
                mPageInput.Text = "1";
                return;
            }

            PDFView.PDFPos pos = new PDFView.PDFPos();
            pos.pageno = page;
            pos.x = x;
            pos.y = y;
            if (m_view.GetType().Equals(typeof(PDFViewVert)) || m_view.GetType().Equals(typeof(PDFViewHorz)))
                m_view.vSetPos(pos, 0, 0);
            else
                m_view.vGotoPage(pos.pageno);
            mPageInput.Text = (pos.pageno + 1).ToString();
        }



        private void OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (mAnnotType != AnnotControl.AnnotType.TypeNone)
            {
                switch (mAnnotType)
                {
                    case AnnotControl.AnnotType.TypeEllipse:
                        m_view.vEllipseEnd();
                        break;
                    case AnnotControl.AnnotType.TypeLine:
                        m_view.vInkEnd();
                        break;
                    case AnnotControl.AnnotType.TypeRect:
                        m_view.vRectEnd();
                        break;
                    case AnnotControl.AnnotType.TypeText:
                        m_view.vNoteEnd();
                        break;
                    case AnnotControl.AnnotType.TypeAnnotEditBox:
                        m_view.vTextEditEnd();
                        break;
                }
                doneAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                removeAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                mAnnotControl.ResetLayout();
                mAnnotType = AnnotControl.AnnotType.TypeNone;
            }
            mPDFView.Width = e.NewSize.Width;
            mPDFView.Height = e.NewSize.Height;
            if (e.PreviousSize.Height > 0 && e.PreviousSize.Width > 0)
            {
                PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                m_view.vSetPos(pos, 0, 0);
            }
        }

        private void initOptionPanels()
        {
            mAnnotControl = new AnnotControl(m_doc.CanSave);
            mAnnotControl.OnButtonClick += OnAnnotItemTapped;
            mSearchPanel = new SearchControl();
            mSearchPanel.OnButtonTapped += OnSearchItemTapped;
            mEditAnnotControl = new EditAnnotControl();
            mEditAnnotControl.OnButtonClick += OnEditAnnotButtonTapped;
            mRichmediaControl = new RichMediaControl();
            mRichmediaControl.OnButtonClick += OnRichMediaButtonTapped;
        }

        private void OnRichMediaButtonTapped(int btnCode)
        {
            switch (btnCode)
            {
                case 0:
                    //play
                    mMediaElement.Play();
                    break;
                case 1:
                    //pause
                    mMediaElement.Pause();
                    break;
                case 2:
                    //stop
                    mMediaElement.Stop();
                    PDFOptionPanel.Children.Clear();
                    mParent.Children.Remove(mMediaElement);
                    mAppBar.IsOpen = false;
                    break;
            }
        }

        private void OnEditAnnotButtonTapped(int btnCode)
        {
            PDFOptionPanel.Children.Clear();
            PDFOptionPanel.Children.Add(mAnnotControl);
            mAppBar.IsOpen = false;
            switch (btnCode)
            {
                case 0:
                    //open annot
                    m_view.vAnnotPerform();
                    break;
                case 1:
                    //remove annot
                    m_view.vAnnotRemove();
                    mIsModified = true;
                    break;
                case 2:
                    //cancel
                    m_view.vAnnotEnd();
                    break;
            }
        }

        private void OnSearchItemTapped(int btnCode, string searchKey, bool matchCase, bool matchWholeWord)
        {
            if (searchKey.Length == 0)
            {
                m_view.vFindEnd();
                return;
            }
            if (!searchKey.Equals(mSearchKey))
            {
                mSearchKey = searchKey;
                m_view.vFindStart(mSearchKey, matchCase, matchWholeWord);
            }
            switch (btnCode)
            {
                case 0:
                    //search prev
                    m_view.vFind(-1);
                    break;
                case 1:
                    //search next
                    m_view.vFind(1);
                    break;
                case -1:
                    //cancel
                    m_view.vFindEnd();
                    break;
            }
        }

        private void OnAnnotItemTapped(AnnotControl.AnnotType type)
        {
            switch (type)
            {
                case AnnotControl.AnnotType.TypeLine:
                    m_view.vInkStart();
                    break;
                case AnnotControl.AnnotType.TypeRect:
                    m_view.vRectStart();
                    break;
                case AnnotControl.AnnotType.TypeEllipse:
                    m_view.vEllipseStart();
                    break;
                case AnnotControl.AnnotType.TypeText:
                    m_view.vNoteStart();
                    break;
                case AnnotControl.AnnotType.TypeAnnotEditBox:
                    m_view.vTextEditStart();
                    break;
                default:
                    break;
            }
            mAppBar.IsOpen = false;
            doneAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Visible;
            removeAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Visible;
            mAnnotType = type;
        }

        private void OnOptionItemTapped(object sender, TappedRoutedEventArgs e)
        {
            m_view.vSelEnd(); //Nermeen clear selection if any
            mSelectIcon.Foreground = new SolidColorBrush(Colors.Black);

            mAppBar.Visibility = Visibility.Visible;
            Button button = sender as Button;
            String name = button.Name;
            if (name.Equals("viewAnnotBtn"))
            {
                PDFOptionPanel.Children.Clear();
                PDFOptionPanel.Children.Add(mAnnotControl);
                mAppBar.IsOpen = true;
            }
            else if (name.Equals("searchBtn"))
            {
                PDFOptionPanel.Children.Clear();
                PDFOptionPanel.Children.Add(mSearchPanel);
                mAppBar.IsOpen = true;
            }
            else if (name.Equals("doneAnnotBtn"))
            {
                mIsModified = true;
                mAnnotControl.ResetLayout();
                switch (mAnnotType)
                {
                    case AnnotControl.AnnotType.TypeLine:
                        m_view.vInkEnd();
                        break;
                    case AnnotControl.AnnotType.TypeRect:
                        m_view.vRectEnd();
                        break;
                    case AnnotControl.AnnotType.TypeEllipse:
                        m_view.vEllipseEnd();
                        break;
                    case AnnotControl.AnnotType.TypeText:
                        m_view.vNoteEnd();
                        break;
                    case AnnotControl.AnnotType.TypeAnnotEditBox:
                        m_view.vTextEditEnd();
                        break;
                    default:
                        break;
                }
                doneAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                removeAnnotBtn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                mAnnotType = AnnotControl.AnnotType.TypeNone;
            }
            else if (name.Equals("removeAnnotBtn"))
            {
                mAnnotControl.ResetLayout();
                switch (mAnnotType)
                {
                    case AnnotControl.AnnotType.TypeLine:
                        m_view.vInkCancel();
                        break;
                    case AnnotControl.AnnotType.TypeRect:
                        m_view.vRectCancel();
                        break;
                    case AnnotControl.AnnotType.TypeEllipse:
                        m_view.vEllipseCancel();
                        break;
                    case AnnotControl.AnnotType.TypeText:
                        m_view.vNoteCancel();
                        break;
                    case AnnotControl.AnnotType.TypeAnnotEditBox:
                        m_view.vTextEditCancel();
                        break;
                    default:
                        break;
                }
                doneAnnotBtn.Visibility = Visibility.Collapsed;
                removeAnnotBtn.Visibility = Visibility.Collapsed;
                mAnnotType = AnnotControl.AnnotType.TypeNone;
            }

            else if (name.Equals(viewMenuBtn.Name))
            {
                if (mMenuView.Visibility == Visibility.Collapsed)
                {
                    mMenuView.ItemsSource = mMenuData.Items;
                    mMenuView.Visibility = Visibility.Visible;
                }
                else
                    mMenuView.Visibility = Visibility.Collapsed;
            }

            else if (name.Equals(viewInfoBtn.Name))
            {
                InformationControl infoDialog = new InformationControl();
                infoDialog.show();
            }

            else if (name.Equals(settingsBtn.Name))
            {
                //LostFocus -= PDFReaderPage_LostFocus;
                SettingsControl settingsControl = new SettingsControl();
                settingsControl.OnViewModeSelected += OnViewModeDialogClose;
                settingsControl.show();
            }
            else if (name.Equals("selectBtn")) //Nermeen handle select button
            {
                m_view.vSelStart();
                mSelectIcon.Foreground = new SolidColorBrush(Colors.Red);
            }
        }


        private void OnBtnGoBack(Object sender, RoutedEventArgs e)
        {
            //mIsMenuTapped = true;
            OnClose(null, null);
        }

        private void OnClose(Object sender, CoreWindowEventArgs e)
        {
            if (m_doc == null || m_view == null)
            {
                m_stream.Dispose();
                Frame.Navigate(typeof(MainPage));
            }
            else if ((mIsModified || m_view.vIsModified()) && m_doc.CanSave)
            {
                if (mSaveDocControl == null)
                {
                    mSaveDocControl = new SaveDocControl();
                    mSaveDocControl.onSaveDialogClose += OnSaveDialogClose;
                }
                mSaveDocControl.show();
            }
            else
                vClose();
        }

        private void OnSaveDialogClose(int button)
        {
            switch (button)
            {
                case 0:
                    //OK
                    m_doc.Save();
                    vClose();
                    break;
                case 1:
                    //No
                    vClose();
                    break;
                default:
                    //cancel
                    break;
            }
            mSaveDocControl.dismiss();
            mSaveDocControl = null; //Nermeen
        }

        private void vClose()
        {
            if (m_stream != null)
            {
                if (m_view != null)
                {
                    PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                    if (OnPageClose != null)
                        OnPageClose(pos);
                    m_view.vClose();
                    m_view = null;
                }

                m_doc.Close();
                m_doc = null;
                m_stream.Dispose();
                Frame.Navigate(typeof(MainPage));
            }
        }

        //private void backButton_Click(object sender, RoutedEventArgs e)
        //{

        //}

        private void OnMenuListItemClicked(object sender, ItemClickEventArgs e)
        {
            MenuItem clickedItem = e.ClickedItem as MenuItem;
            if (clickedItem != null)
            {
                int page = clickedItem.Page;
                m_view.vGotoPage(page);
            }
        }



        public void OnPDFPageChanged(int pageno)
        {
            //if(!isPDFZooming)
            mPageInput.Text = (pageno + 1).ToString();
        }

        public bool OnPDFSingleTapped(float x, float y)
        {
            return false;
        }

        public void OnPDFLongPressed(float x, float y)
        {
        }

        public void OnPDFFound(bool found)
        {
        }

        public void OnPDFPageDisplayed(Canvas canvas, PDFVPage vpage)
        {
        }

        public void OnPDFSelecting(Canvas canvas, PDFRect rect1, PDFRect rect2)
        {
        }

        public string Between(string STR, string FirstString, string LastString)
        {
            string FinalString;
            int Pos1 = STR.IndexOf(FirstString) + FirstString.Length;
            int Pos2 = STR.IndexOf(LastString);
            FinalString = STR.Substring(Pos1, Pos2 - Pos1);
            return FinalString;
        }


        public void OnPDFSelected() //Nermeen show selection handler dialog
        {
            if (m_view == null || m_view.vSelGetText() == null)
                return;
            mTextSelectionDialog = new TextSelectionControl(m_view.vSelGetText());
            mTextSelectionDialog.OnCloseDialog += OnCloseSelectDialog;
            mTextSelectionDialog.show();
        }

        private void OnCloseSelectDialog(string selection, int selectType, bool cancel) //Nermeen selection dialog listener
        {
            if (!cancel)
            {
                if (selectType == 3) //clipboard
                {
                    if (m_view == null)
                        return;
                    var dataPackage = new DataPackage();
                    dataPackage.SetText(selection);
                    Clipboard.SetContent(dataPackage);
                    ShowToastNotification("Text copied to clipboard", m_view.vSelGetText());
                }
                else
                {
                    uint color = 0xFFFFFF00;
                    if (selectType == 1)
                        color = 0xFF0000C0;
                    else if (selectType == 2)
                        color = 0xFFC00000;
                    else if (selectType == 4)
                        color = 0xFF00C000;
                    bool result = m_view.vSelMarkup(color, selectType);
                    if (result)
                        mIsModified = true;
                    else
                        ShowToastNotification("Warning", "Add annotation failed!");
                }
                m_view.vSelEnd();
                mSelectIcon.Foreground = new SolidColorBrush(Colors.Black);
            }
            else
                m_view.vSelCancel();
            mTextSelectionDialog.dismiss();
            mTextSelectionDialog = null;
        }

        public void OnPDFAnnotClicked(PDFPage page, PDFAnnot annot)
        {
            if (annot == null)
                return;

            if (m_doc.CanSave && annot.EditType > 0) //TODO form edit-box.
            {
                mFormEditControl = new FormEditControl();
                mFormEditControl.Annot = annot;
                mFormEditControl.OnEditDialogDialogClose += onFormEditEnd;
                mFormEditControl.Show();
                PDFOptionPanel.Children.Clear();
                PDFOptionPanel.Children.Add(mEditAnnotControl);
                mAppBar.IsOpen = true;
            }
            else if (m_doc.CanSave && annot.ComboItemCount >= 0) //TODO form choice
            {
                mFormComboControl = new FormComboControl();
                mFormComboControl.Annot = annot;
                mFormComboControl.OnComboDialogDialogClose += onFormComboEnd;
                mFormComboControl.Show();
            }
            else if (annot != null)
            {
                PDFOptionPanel.Children.Clear();
                PDFOptionPanel.Children.Add(mEditAnnotControl);
                mAppBar.IsOpen = true;
            }
        }

        private void onFormEditEnd(bool canceled, String text)
        {
            if (!canceled)
            {
                mFormEditControl.Annot.EditText = text;
                mIsModified = true;
            }
            mFormEditControl.OnEditDialogDialogClose -= onFormEditEnd;
            mFormEditControl.Dismiss();
            mFormEditControl.Annot = null;
            mFormEditControl = null;
            PDFView.PDFPos pos = m_view.vGetPos(0, 0);
            PDFVPage vpage = m_view.vGetPage(pos.pageno);
            m_view.vRenderAsync(vpage);
            m_view.vAnnotEnd();
        }

        private void onFormComboEnd(bool canceled, int comboIndex)
        {
            if (!canceled && comboIndex != -1)
            {
                mFormComboControl.Annot.ComboItemSel = comboIndex;
                mIsModified = true;
            }
            mFormComboControl.OnComboDialogDialogClose -= onFormComboEnd;
            mFormComboControl.Dismiss();
            mFormComboControl.Annot = null;
            mFormComboControl = null;
            PDFView.PDFPos pos = m_view.vGetPos(0, 0);
            PDFVPage vpage = m_view.vGetPage(pos.pageno);
            m_view.vRenderSync(vpage);
            m_view.vAnnotEnd();
        }

        public void OnPDFAnnotEnd()
        {
        }

        public void OnPDFAnnotGoto(int pageno)
        {
            m_view.vGotoPage(pageno);
        }

        public async void OnPDFAnnotURI(string uri)
        {
            if (!uri.StartsWith("www."))
                uri = "http://www." + uri;
            if (!uri.StartsWith("http://"))
                uri = "http://" + uri;
            Uri url = new Uri(uri);
            await Launcher.LaunchUriAsync(url);
        }

        public async Task OnPDFAnnotRichMediaAsync(PDFAnnot annot)
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
                        StorageFolder videoFolder = ApplicationData.Current.LocalFolder;
                        StorageFolder folder = await videoFolder.CreateFolderAsync("temp", CreationCollisionOption.OpenIfExists);
                        String path = folder.Path;
                        path = path + "\\" + movie;
                        if (annot.GetRichMediaItemSourceData(0, path))
                        {
                            playMedia(path);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);

            }
        }

        private void playMedia(String path)
        {
            mMediaElement = new MediaElement();
            mMediaElement.Source = new Uri(path);
            mParent.Children.Add(mMediaElement);
            Grid.SetRowSpan(mMediaElement, 2);
            mMediaElement.Play();
            mMediaElement.MediaEnded += onMediaEnded;
            mAppBar.Visibility = Visibility.Visible;
            PDFOptionPanel.Children.Clear();
            PDFOptionPanel.Children.Add(mRichmediaControl);
            mAppBar.IsOpen = true;
        }

        private void onMediaEnded(object sender, RoutedEventArgs e)
        {
            OnRichMediaButtonTapped(2/*stop*/);
        }

        public void OnPDFAnnotPopup(PDFAnnot annot, string subj, string text)
        {
            mAnnot = annot;
            mTextAnnotDialog = new TextAnnotControl();
            if (annot != null)
                mTextAnnotDialog.SetContent(annot.PopupSubject, annot.PopupText);
            mTextAnnotDialog.OnCloseDialog += OnCloseDialog;
            mTextAnnotDialog.show();
        }

        private void OnCloseDialog(String subject, String content, bool cancel, bool edit)
        {
            if (cancel)
            {
                m_view.vNoteRemoveLast();
                return;
            }
            if (mAnnot == null)
            {
                int index = -1;
                PDFView.PDFPos pos = m_view.vGetPos(0, 0);
                PDFVPage vpage = m_view.vGetPage(pos.pageno);
                if (vpage != null)
                {
                    PDFPage page = vpage.GetPage();
                    if (page != null)
                    {
                        index = page.AnnotCount;
                        if (index > 0)
                            mAnnot = page.GetAnnot(index - 1);
                        if (mAnnot == null)
                            return;
                    }
                }
            }
            mAnnot.PopupSubject = subject;
            mAnnot.PopupText = content;
            mTextAnnotDialog.dismiss();
            mTextAnnotDialog = null;
        }

        private void OnKeyDown(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                try
                {
                    int page = Convert.ToInt32(mPageInput.Text);
                    m_view.vGotoPage(--page);
                }
                catch
                {
                }
            }
        }

        public async void OnPDFAnnotRemoteDest(string dest)
        {
            string filename = dest;
            string pagenumber = "";
            char[] separator = { '/' };
            string[] elements = dest.Split(separator);

            long elementscount = elements.Length;


            if (elements[elementscount - 1] != null) // page number is not null
            {
                pagenumber = "/" + elements[elementscount - 1];
            }
            else
            {
                pagenumber = "/0";
            }

            if (filename.EndsWith(pagenumber))
            {
                filename = filename.Substring(0, filename.LastIndexOf(pagenumber));
            }

            if (pagenumber == "/0") // page number isn't set
            {
                // handle file without goto page
                // simple example below, it works only with absolute file paths

                // convert / (Linux) to \ (Windows)
                filename = filename.Replace("/", "\\");

                Uri url = new System.Uri(filename, UriKind.RelativeOrAbsolute);
                await Windows.System.Launcher.LaunchUriAsync(url);
            }
            else
            {
                // todo handle spawn new reader and goto page number
                // filename: new pdf file
                // pagenumber: page number
            }

        }
        public async void OnPDFAnnotFileLink(string filelink)
        {
            // handle file without goto page
            // simple example below
            Uri url = new Uri(filelink);
            await Windows.System.Launcher.LaunchUriAsync(url);
        }

        private void OnThumbItemTapped(object sender, TappedRoutedEventArgs e)
        {
            Frame.Navigate(typeof(PDFThumbPage), m_doc);
        }

        public void OnPDFPageTapped(PDFVPage vpage)
        {
        }
        private void ShowToastNotification(string title, string stringContent)
        {
            try
            {
                ToastNotifier ToastNotifier = ToastNotificationManager.CreateToastNotifier();
                Windows.Data.Xml.Dom.XmlDocument toastXml = ToastNotificationManager.GetTemplateContent(ToastTemplateType.ToastText02);
                Windows.Data.Xml.Dom.XmlNodeList toastNodeList = toastXml.GetElementsByTagName("text");
                if (title != null && title.Length > 0)
                    toastNodeList.Item(0).AppendChild(toastXml.CreateTextNode(title));
                toastNodeList.Item(1).AppendChild(toastXml.CreateTextNode(stringContent));
                Windows.Data.Xml.Dom.IXmlNode toastNode = toastXml.SelectSingleNode("/toast");
                Windows.Data.Xml.Dom.XmlElement audio = toastXml.CreateElement("audio");
                audio.SetAttribute("src", "ms-winsoundevent:Notification.SMS");

                ToastNotification toast = new ToastNotification(toastXml);
                toast.ExpirationTime = DateTime.Now.AddSeconds(3);
                ToastNotifier.Show(toast);
            }
            catch (Exception)
            {

            }
        }

        //public void OnPDFZoomStart()
        //{
        //    isPDFZooming = true;
        //}

        //public void OnPDFZoomEnd()
        //{
        //    isPDFZooming = false;
        //}
    }
}