#include "NtService.h"
#include "Utf8.h"

#include <cassert>
#include <windows.h>

#pragma warning( disable : 4355 )


namespace
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // simple RAII class to manage SC_HANDLE's
    //
    class ServiceHandle
    {
    public:
        explicit ServiceHandle(SC_HANDLE h) : h_(h) {}
        ~ServiceHandle() { if (h_) { CloseServiceHandle(h_); } }
        operator SC_HANDLE() const { return h_; }
    private:
        SC_HANDLE h_;
    };
}


///////////////////////////////////////////////////////////////////////////////////////////////////
mvd::NtService::ExceptionStartedAsNormalApplication::ExceptionStartedAsNormalApplication() 
    : std::runtime_error("mvd::NtService::ExceptionStartedAsNormalApplication")
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////
class mvd::NtService::Impl
{
public:
    Impl(mvd::NtService* self);
    ~Impl();

    auto StartAsService() -> void;
    auto StartFromCommandLine() -> void;
    auto SetStatus(DWORD status, DWORD error = 0) -> void;

private:
    static auto WINAPI StaticServiceMain(DWORD argc, wchar_t* argv[]) -> void;
    auto ServiceMain(DWORD argc, wchar_t* argv[]) -> void;
    static auto WINAPI ControlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) -> DWORD;

public:
    // this is ONLY used by the StaticServiceMain callback
    static mvd::NtService::Impl* gService;

private:
    mvd::NtService*         self_;
    SERVICE_STATUS_HANDLE   status_;
    unsigned int            status_code_;
    unsigned int            error_code_;
};


// initialise statics
mvd::NtService::Impl* mvd::NtService::Impl::gService = 0;


///////////////////////////////////////////////////////////////////////////////////////////////////
mvd::NtService::Impl::Impl(mvd::NtService* self)
    : self_(self)
    , status_(0)
    , status_code_(SERVICE_STOPPED)
    , error_code_(0)
{
    assert(!gService);
    gService = this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
mvd::NtService::Impl::~Impl()
{
    assert(gService);
    gService = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// this will start the app as a true service.  If the call to the dispatcher fails, then check for error code [1063] as 
// this indicates that we are running as a normal application (not an NT service).  Useful for debugging
// 
auto mvd::NtService::Impl::StartAsService() -> void
{
    // grab a local copy of our service name
    std::wstring const name = mvd::widen(self_->GetShortName());
    
    // build the service table
    SERVICE_TABLE_ENTRY entries[2];
    entries[0].lpServiceName = const_cast<wchar_t*>(name.c_str());
    entries[0].lpServiceProc = StaticServiceMain;
    entries[1].lpServiceName = 0;
    entries[1].lpServiceProc = 0;

    // and start the dispatcher
    if (!StartServiceCtrlDispatcherW(entries))
    {
        DWORD const error = GetLastError();
        if (error == 1063)
        {
            // failed to connect to the SCM.  most likely running as a stand alone app rather than a service
            throw ExceptionStartedAsNormalApplication();
        }
        else
        {
            throw std::runtime_error("mvd::NtService::Impl::StartAsService() - StartServiceCtrlDispatcher() returned error code [" + std::to_string(error) + "]");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// run as a normal application (not an NT service)
//
auto mvd::NtService::Impl::StartFromCommandLine() -> void
{
    self_->MainLoop();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Impl::SetStatus(DWORD status, DWORD error) -> void
{
    status_code_ = status;
    error_code_ = error;

    SERVICE_STATUS ss;
    ss.dwServiceType                = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState               = status_code_;
    ss.dwControlsAccepted           = SERVICE_ACCEPT_STOP;
    ss.dwWin32ExitCode              = error_code_ ? ERROR_SERVICE_SPECIFIC_ERROR : NO_ERROR;
    ss.dwServiceSpecificExitCode    = error_code_;
    ss.dwCheckPoint                 = 0;
    ss.dwWaitHint                   = 1000;
    
    SetServiceStatus(status_, &ss);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Impl::StaticServiceMain(DWORD argc, wchar_t* argv[]) -> void
{
    assert(gService);
    if (gService) { gService->ServiceMain(argc, argv); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Impl::ServiceMain(DWORD argc, wchar_t* argv[]) -> void
{
    // register our control handler    
    status_ = RegisterServiceCtrlHandlerExW(mvd::widen(self_->GetShortName()).c_str(), &Impl::ControlHandlerEx, this);
    if (!status_)
    {
        SetStatus(SERVICE_STOPPED, -1);
        return;
    }

    SetStatus(SERVICE_RUNNING);
    try { self_->MainLoop();} catch (...) { assert(false); }
    SetStatus(SERVICE_STOPPED);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Impl::ControlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) -> DWORD
{
    NtService::Impl* impl = reinterpret_cast<NtService::Impl*>(lpContext);
    switch(dwControl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:      
        impl->SetStatus(SERVICE_STOP_PENDING); 
        impl->self_->Shutdown();   
        break;

    case SERVICE_CONTROL_INTERROGATE:   
        impl->SetStatus(impl->status_code_, impl->error_code_);
        break;

    // and we'll ignore the rest for now...
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_PARAMCHANGE:
    case SERVICE_CONTROL_NETBINDADD:
    case SERVICE_CONTROL_NETBINDREMOVE:
    case SERVICE_CONTROL_NETBINDENABLE:
    case SERVICE_CONTROL_NETBINDDISABLE:
    case SERVICE_CONTROL_DEVICEEVENT:
    case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
    case SERVICE_CONTROL_POWEREVENT:
    case SERVICE_CONTROL_SESSIONCHANGE:
    case SERVICE_CONTROL_PRESHUTDOWN:
    case SERVICE_CONTROL_TIMECHANGE:
    case SERVICE_CONTROL_TRIGGEREVENT:
    default:
        ;
    }
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////
mvd::NtService::NtService() : impl_(new Impl(this)) {}
mvd::NtService::~NtService() {}
auto mvd::NtService::StartAsService()                                       -> void { impl_->StartAsService();          }
auto mvd::NtService::StartFromCommandLine()                                 -> void { impl_->StartFromCommandLine();    }
auto mvd::NtService::SetStatus(unsigned long status, unsigned long error)   -> void { impl_->SetStatus(status, error);  }

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Register() -> void
{
    auto const hSCM = ServiceHandle{ OpenSCManagerW(0, 0, SC_MANAGER_CREATE_SERVICE) };
    if (!hSCM) 
    {
        DWORD const error = GetLastError();
        assert(false);
        throw std::runtime_error("mvd::NtService::Impl::Register() - Unable to open the Service Control Manager.  OpenSCManagerW() returned error code [" + std::to_string(error) + "]");
    }

    wchar_t filename[MAX_PATH];
    if (GetModuleFileNameW(GetModuleHandleW(NULL), filename, MAX_PATH) == 0)
    {
        DWORD const error = GetLastError();
        assert(false);
        throw std::runtime_error("mvd::NtService::Impl::Register() - Unable to get our module filename.  GetModuleFileNameW() returned error code [" + std::to_string(error) + "]");
    }

    ServiceHandle hService(
        CreateServiceW(
            hSCM,
            mvd::widen(GetShortName()).c_str(),
            mvd::widen(GetFullName()).c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, 
            filename,
            0, 
            0, 
            0,  //mszDependencies, 
            0, 
            0));

    if (!hService)
    {
        DWORD error = GetLastError();
        assert(false);
        throw std::runtime_error("mvd::NtService::Register() - Unable to create the service.  CreateServiceW() returned error code [" + std::to_string(error) + "]");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mvd::NtService::Unregister() -> void
{
    ServiceHandle hSCM(OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE));
    if (!hSCM) 
    { 
        DWORD const error = GetLastError();
        assert(false);
        throw std::runtime_error("mvd::NtService::Unregister() - Unable to open the Service Control Manager.  OpenSCManagerW() returned error [" + std::to_string(error) + "]");
    }

    ServiceHandle hService(OpenServiceW(hSCM, mvd::widen(GetShortName()).c_str(), SERVICE_ALL_ACCESS));
    if (!hService)
    {
        DWORD const error = GetLastError();
        if (error == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            return;
        }
        else
        {
            assert(false);
            throw std::runtime_error("mvd::NtService::Unregister() - Unable to open the service.  OpenServiceW() returned error [" + std::to_string(error) + "]");
        }
    }
    else
    {
        if (!DeleteService(hService))
        {
            DWORD const error = GetLastError();
            assert(false);
            throw std::runtime_error("mvd::NtService::Unregister() - Unable to delete the service.  DeleteService() returned error [" + std::to_string(error) + "]");
        }
    }
}



