using PDFReader;
using PDFViewerSDK_Win10.view;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using Windows.Storage;
using Windows.Storage.AccessCache;
using Windows.Storage.Pickers;
using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace PDFViewerSDK_Win10
{
    public sealed class Item
    {
        String _Title;
        event PropertyChangedEventHandler _PropertyChanged;

        public Item(String token)
        {
            _token = token;
        }
        public void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventArgs pcea = new PropertyChangedEventArgs(propertyName);
            _PropertyChanged(this, pcea);
        }
        //Title
        public String Title
        {
            get
            {
                return _Title;
            }
            set
            {
                _Title = value;
                //OnPropertyChanged("Title");
            }
        }
        public String get_token()
        {
            return _token;
        }
        private String _token;
    };

    [Windows.Foundation.Metadata.WebHostHiddenAttribute]
    public sealed class RecentListData
    {
        private List<Item> _items;
        public RecentListData()
        {
            _items = new List<Item>();
            Uri _baseUri = new Uri("ms-appx:///");
            //String LONG_LOREM_IPSUM = "";//"Curabitur class aliquam vestibulum nam curae maecenas sed integer cras phasellus suspendisse quisque donec dis praesent accumsan bibendum pellentesque condimentum adipiscing etiam consequat vivamus dictumst aliquam duis convallis scelerisque est parturient ullamcorper aliquet fusce suspendisse nunc hac eleifend amet blandit facilisi condimentum commodo scelerisque faucibus aenean ullamcorper ante mauris dignissim consectetuer nullam lorem vestibulum habitant conubia elementum pellentesque morbi facilisis arcu sollicitudin diam cubilia aptent vestibulum auctor eget dapibus pellentesque inceptos leo egestas interdum nulla consectetuer suspendisse adipiscing pellentesque proin lobortis sollicitudin augue elit mus congue fermentum parturient fringilla euismod feugiat";
        }
        public List<Item> Items
        {
            get
            {
                return _items;
            }
        }
        internal void add(String name, String token)
        {
            Item item = new Item(token);
            item.Title = name;
            _items.Add(item);
        }
    };

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        static private string mFileToken;
        static public string FileToken
        {
            get
            {
                return mFileToken;
            }
        }

        public MainPage()
        {
            this.InitializeComponent();
            PDFReaderPage.OnPageClose += OnDocClose;
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            if (e.Parameter != null)
            {
                StorageFile file = e.Parameter as StorageFile;
                if (file != null)
                {
                    if (StorageApplicationPermissions.MostRecentlyUsedList.Entries.Count >= 25)
                    {
                        RemoveLastFile();
                    }
                    mFileToken = StorageApplicationPermissions.MostRecentlyUsedList.Add(file);
                    IRandomAccessStream stream = await file.OpenAsync(FileAccessMode.ReadWrite);
                    if (stream != null)
                    {
                        Frame.Navigate(typeof(PDFReaderPage), stream);
                    }
                }
            }
            UpdateRecentList();
        }

        private void OnDocClose(PDFView.PDFPos pos)
        {

            if (!ApplicationData.Current.LocalSettings.Values.ContainsKey(mFileToken + "_page"))
            {
                ApplicationData.Current.LocalSettings.Values.Add(mFileToken + "_page", pos.pageno);
            }
            else
            {
                ApplicationData.Current.LocalSettings.Values[mFileToken + "_page"] = pos.pageno;
            }

            if (!ApplicationData.Current.LocalSettings.Values.ContainsKey(mFileToken + "_x"))
            {
                ApplicationData.Current.LocalSettings.Values.Add(mFileToken + "_x", pos.x);
            }
            else
            {
                ApplicationData.Current.LocalSettings.Values[mFileToken + "_x"] = pos.x;
            }

            if (!ApplicationData.Current.LocalSettings.Values.ContainsKey(mFileToken + "_y"))
            {
                ApplicationData.Current.LocalSettings.Values.Add(mFileToken + "_y", pos.y);
            }
            else
            {
                ApplicationData.Current.LocalSettings.Values[mFileToken + "_y"] = pos.y;
            }

        }

        private void About_Click(Object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(AboutPage));
        }

        async private void OnFileListItemClicked(Object sender, ItemClickEventArgs e)
        {
            Item clickedItem = e.ClickedItem as Item;
            if (clickedItem != null)
            {
                String token = clickedItem.get_token();
                StorageFile file = await StorageApplicationPermissions.MostRecentlyUsedList.GetFileAsync(token);
                if (file != null)
                {
                    mFileToken = token;
                    IRandomAccessStream stream = await file.OpenAsync(FileAccessMode.ReadWrite);
                    if (stream != null)
                    {
                        Frame.Navigate(typeof(PDFReaderPage), stream);
                    }
                }
            }
        }

        private void RemoveLastFile()
        {
            AccessListEntryView list = StorageApplicationPermissions.MostRecentlyUsedList.Entries;
            AccessListEntry first = list.First<AccessListEntry>();
            String token = first.Token;
            StorageApplicationPermissions.MostRecentlyUsedList.Remove(token);
            if (ApplicationData.Current.LocalSettings.Values.ContainsKey(token))
            {
                ApplicationData.Current.LocalSettings.Values.Remove(token);
                if (mFileToken.Equals(token))
                    mFileToken = string.Empty;
            }
        }

        async private void UpdateRecentList()
        {
            //Load recent used files
            AccessListEntryView list = StorageApplicationPermissions.MostRecentlyUsedList.Entries;
            List<AccessListEntry> recentFileList = list.ToList();
            if (recentFileList.Count == 0)
            {
                mEmptyListHint.Visibility = Visibility.Visible;
            }
            else
            {
                mEmptyListHint.Visibility = Visibility.Collapsed;
                RecentListData data = new RecentListData();
                foreach (AccessListEntry entry in recentFileList)
                {
                    try
                    {
                        StorageFile file = await StorageApplicationPermissions.MostRecentlyUsedList.GetFileAsync(entry.Token);
                        if (file != null)
                        {
                            data.add(file.Name, entry.Token);
                        }
                    }
                    catch (Exception)
                    {
                    }
                }
                FileList.ItemsSource = data.Items;
            }
        }

        async private void BrowsFileButton_Click(Object sender, RoutedEventArgs e)
        {
            FileOpenPicker filePicker;
            filePicker = new FileOpenPicker();
            filePicker.ViewMode = PickerViewMode.List;
            filePicker.SuggestedStartLocation = PickerLocationId.DocumentsLibrary;
            filePicker.FileTypeFilter.Add(".PDF");
            filePicker.FileTypeFilter.Add(".pdf");
            StorageFile file = await filePicker.PickSingleFileAsync();
            if (file != null)
            {
                // Application now has read/write access to the picked file
                IRandomAccessStream stream = await file.OpenAsync(FileAccessMode.ReadWrite);
                if (stream != null)
                {
                    mFileToken = StorageApplicationPermissions.MostRecentlyUsedList.Add(file);
                    Frame.Navigate(typeof(PDFReaderPage), stream);
                }
            }
            else
            {
            }
        }
    }
}
