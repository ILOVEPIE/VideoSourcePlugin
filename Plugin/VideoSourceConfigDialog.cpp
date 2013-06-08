#include "VideoSourceConfigDialog.h"
#include "VideoSourcePlugin.h"

#include "WindowsHelper.h"
#include "resource.h"

#define HANDLE_DEFAULT default: return false

INT_PTR CALLBACK Config_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Config_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

VideoSourceConfigDialog::VideoSourceConfigDialog(VideoSourceConfig *config)
{
    this->config = config;
}

VideoSourceConfigDialog::~VideoSourceConfigDialog()
{

}

bool VideoSourceConfigDialog::Show()
{
    return DialogBoxParam(VideoSourcePlugin::hinstDLL, MAKEINTRESOURCE(IDD_VIDEOCONFIG), API->GetMainWindow(), Config_DlgProc, (LPARAM)this) == IDOK;
}

INT_PTR CALLBACK Config_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
        HANDLE_MSG      (hwndDlg, WM_INITDIALOG, Config_OnInitDialog);
        HANDLE_MSG      (hwndDlg, WM_COMMAND,    Config_OnCommand);
	    HANDLE_DEFAULT;	
    }
}



BOOL Config_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lParam);
    VideoSourceConfigDialog *_this = reinterpret_cast<VideoSourceConfigDialog *>(lParam);
    VideoSourceConfig *config = _this->GetConfig();

    LocalizeWindow(hwnd);

    _this->hwndPathOrUrl             = GetDlgItem(hwnd, IDC_PATH_OR_URL);
    _this->hwndWidth                 = GetDlgItem(hwnd, IDC_WIDTH);
    _this->hwndHeight                = GetDlgItem(hwnd, IDC_HEIGHT);
    _this->hwndVolume                = GetDlgItem(hwnd, IDC_VOLUME);
    _this->hwndStretch               = GetDlgItem(hwnd, IDC_STRETCH);
    _this->hwndIsAudioOutputToStream = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_STREAM);
    _this->hwndIsAudioOutputToDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_DEVICE);
    _this->hwndAudioOutputType       = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TYPE);
    _this->hwndAudioOutputDevice     = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);

    Edit_SetText(_this->hwndPathOrUrl,  config->pathOrUrl.Array());
    Edit_SetText(_this->hwndWidth,      IntString(config->width).Array());
    Edit_SetText(_this->hwndHeight,     IntString(config->height).Array());

    Button_SetCheck(_this->hwndStretch, config->isStretching);
    Slider_SetRange(_this->hwndVolume, 0, 100);
    Slider_SetPos(_this->hwndVolume, config->volume);


    int index = -1;

    auto audioOutputTypes = config->GetAudioOutputTypes();

    for(auto i = audioOutputTypes.begin(); i < audioOutputTypes.end(); i++) {
        AudioOutputType &audioOutputType = *i;
        ComboBox_AddString(_this->hwndAudioOutputType, audioOutputType.GetDescription().Array());
        if (audioOutputType.GetName() == config->audioOutputType) {
            index = (int)(i - audioOutputTypes.begin());
        }
    }

    if (index < 0) {
        index = 0;                
    }
    
    ComboBox_SetCurSel(_this->hwndAudioOutputType, index);

    AudioOutputType &audioOutputType = config->GetAudioOutputTypes()[index];
    auto audioOutputDevices = audioOutputType.GetAudioOutputDevices();
        
    index = -1;

    if (audioOutputDevices.size()) {
        for(auto i = audioOutputDevices.begin(); i < audioOutputDevices.end(); i++) {
            AudioOutputDevice &audioOutputDevice = *i;
            ComboBox_AddString(_this->hwndAudioOutputDevice, audioOutputDevice.GetName().Array());
            if (audioOutputDevice.GetLongName() == config->audioOutputDevice) {
                index = (int)(i - audioOutputDevices.begin());
            }
        }
             
        if (index < 0) {
            index = 0;                
        }

        ComboBox_SetCurSel(_this->hwndAudioOutputDevice, index);
    }

    Button_SetCheck(_this->hwndIsAudioOutputToStream, config->isAudioOutputToStream);
    Button_SetCheck(_this->hwndIsAudioOutputToDevice, !config->isAudioOutputToStream);

    FORWARD_WM_COMMAND(
        hwnd, 
        config->isAudioOutputToStream ? IDC_AUDIO_OUTPUT_TO_STREAM : IDC_AUDIO_OUTPUT_TO_DEVICE,
        config->isAudioOutputToStream ? _this->hwndIsAudioOutputToStream : _this->hwndIsAudioOutputToDevice,
        BN_CLICKED,
        SendMessage);
    
    return true;
}

void Config_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_AUDIO_OUTPUT_TO_DEVICE:
    case IDC_AUDIO_OUTPUT_TO_STREAM:
        {
            if (codeNotify == BN_CLICKED) {
                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                HWND hwndIsAudioOutputToStream = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_STREAM);
                HWND hwndIsAudioOutputToDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TO_DEVICE);
                HWND hwndAudioOutputType = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_TYPE);
                HWND hwndAudioOutputDevice = GetDlgItem(hwnd, IDC_AUDIO_OUTPUT_DEVICE);

                bool isAudioOutputToStream = id == IDC_AUDIO_OUTPUT_TO_STREAM;

                EnableWindow(hwndAudioOutputType, !isAudioOutputToStream);
                EnableWindow(hwndAudioOutputDevice, !isAudioOutputToStream);
                    
                if (!isAudioOutputToStream) {
                    int index = ComboBox_GetCurSel(_this->hwndAudioOutputType);
                    if (index > 0) {
                        AudioOutputType &type = _this->GetConfig()->GetAudioOutputTypes()[index];
                        EnableWindow(hwndAudioOutputDevice, (type.GetAudioOutputDevices().size()) ? true : false);
                    }
                }
            }
        break;
        }
    case IDC_AUDIO_OUTPUT_TYPE:
        {
            if (codeNotify == CBN_SELCHANGE) {

                VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

                int index = ComboBox_GetCurSel(_this->hwndAudioOutputType);

                ComboBox_ResetContent(_this->hwndAudioOutputDevice);

                AudioOutputType &audioOutputType = _this->GetConfig()->GetAudioOutputTypes()[index];
                auto audioOutputDevices = audioOutputType.GetAudioOutputDevices();
        
                index = -1;

                if (audioOutputDevices.size()) {
                    if (audioOutputDevices.size()) {
                        for(auto i = audioOutputDevices.begin(); i < audioOutputDevices.end(); i++) {
                            AudioOutputDevice &audioOutputDevice = *i;
                            ComboBox_AddString(_this->hwndAudioOutputDevice, audioOutputDevice.GetName().Array());
                            if (audioOutputDevice.GetLongName() == _this->GetConfig()->audioOutputDevice) {
                                index = (int)(i - audioOutputDevices.begin());
                            }
                        }
                    }        

                    if (index < 0) {
                        index = 0;                
                    }

                    ComboBox_SetCurSel(_this->hwndAudioOutputDevice, index);
                    EnableWindow(_this->hwndAudioOutputDevice, true);
                } else {
                    EnableWindow(_this->hwndAudioOutputDevice, false);
                }                    
            }
            break;
        }
    case IDOK:
        {
            VideoSourceConfigDialog *_this = (VideoSourceConfigDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
            VideoSourceConfig *config = _this->GetConfig();

            config->pathOrUrl = GetEditText(_this->hwndPathOrUrl);
            config->width = GetEditText(_this->hwndWidth).ToInt();
            config->height = GetEditText(_this->hwndHeight).ToInt();
            config->volume = Slider_GetPos(_this->hwndVolume);
            config->isStretching = Button_IsChecked(_this->hwndStretch);
            config->isAudioOutputToStream = Button_IsChecked(_this->hwndIsAudioOutputToStream);

            int audioOutputTypeIndex = ComboBox_GetCurSel(_this->hwndAudioOutputType);
            int audioOutputDeviceIndex = ComboBox_GetCurSel(_this->hwndAudioOutputDevice);

            AudioOutputType &type = config->GetAudioOutputTypes()[audioOutputTypeIndex];
            config->audioOutputType = type.GetName();
            if (audioOutputDeviceIndex >= 0) {
                AudioOutputDevice &device = type.GetAudioOutputDevices()[audioOutputDeviceIndex];
                config->audioOutputDevice = device.GetLongName();
            }
            
            EndDialog(hwnd, IDOK);
            break;
        }
    case IDCANCEL:
        {
            EndDialog(hwnd, IDCANCEL);
            break;
        }
    }
}


/*
 * VideoSourceConfigDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( VideoSourceConfigDialog, wxDialog )


/*
 * VideoSourceConfigDialog event table definition
 */

BEGIN_EVENT_TABLE( VideoSourceConfigDialog, wxDialog )

////@begin VideoSourceConfigDialog event table entries
////@end VideoSourceConfigDialog event table entries

END_EVENT_TABLE()


/*
 * VideoSourceConfigDialog constructors
 */

VideoSourceConfigDialog::VideoSourceConfigDialog()
{
    Init();
}

VideoSourceConfigDialog::VideoSourceConfigDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * VideoSourceConfigDialog creator
 */

bool VideoSourceConfigDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin VideoSourceConfigDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end VideoSourceConfigDialog creation
    return true;
}


/*
 * VideoSourceConfigDialog destructor
 */

VideoSourceConfigDialog::~VideoSourceConfigDialog()
{
////@begin VideoSourceConfigDialog destruction
////@end VideoSourceConfigDialog destruction
}


/*
 * Member initialisation
 */

void VideoSourceConfigDialog::Init()
{
////@begin VideoSourceConfigDialog member initialisation
////@end VideoSourceConfigDialog member initialisation
}


/*
 * Control creation for VideoSourceConfigDialog
 */

void VideoSourceConfigDialog::CreateControls()
{    
////@begin VideoSourceConfigDialog content construction
    // Generated by DialogBlocks, 08/06/2013 19:35:02 (unregistered)

    VideoSourceConfigDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Static text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* itemStaticBox4 = new wxStaticBox( itemDialog1, wxID_STATIC, _("Static box"), wxDefaultPosition, wxSize(800, 600), 0 );
    itemBoxSizer2->Add(itemStaticBox4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end VideoSourceConfigDialog content construction
}


/*
 * Should we show tooltips?
 */

bool VideoSourceConfigDialog::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap VideoSourceConfigDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin VideoSourceConfigDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end VideoSourceConfigDialog bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon VideoSourceConfigDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin VideoSourceConfigDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end VideoSourceConfigDialog icon retrieval
}
