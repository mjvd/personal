#pragma once

#include <string>
#include <memory>
#include <stdexcept>

namespace mvd
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Base class to easily create NT service type applications.  Just derive from this class, 
    // override the 4 pure virtual methods, and you're done.
    //
    // Note that although not enforced in code, it only makes sense to create ONE of these objects 
    // in your application.
    //
    // The real guts of the application should go inside the MainLoop() method.  There are two ways
    // to start this application - either as an NT service, or as a normal Windows application.  In 
    // both cases, the start() method is a blocking call.
    //
    // In order to cleanly shut down your application, you must override the Shutdown() method.  This
    // method will be called from a DIFFERENT THREAD than the one that called Startxxx().  For an 
    // NT service, Shutdown() will be called automatically when the service is stopped.  When running
    // as a normal Windows application, the Shutdown() method must be called manually from somewhere
    // else.
    //
    //

    class NtService
    {
    public:
        // special exception to indicate that you tried to start this NT Service application as a 
        //normal Win32 app WITHOUT passing the '-debug' command-line parameter
        //
        struct ExceptionStartedAsNormalApplication : public std::runtime_error 
        { 
            ExceptionStartedAsNormalApplication();
        };

    public:
        NtService();
        NtService(NtService const&) = delete;
        NtService& operator=(NtService const&) = delete;
        ~NtService();

        // register/unregister this app with the service control manager.
        auto Register()     -> void;
        auto Unregister()   ->void;

        // start this application either as an NT service, or as a normal Windows application.  Note
        // that this is a blocking call.
        auto StartAsService()       -> void;
        auto StartFromCommandLine() -> void;

        virtual auto GetShortName() const -> std::string = 0;
        virtual auto GetFullName()  const -> std::string = 0;
        virtual auto Shutdown()           -> void        = 0;

    protected:
        // the real meat of the application goes inside here
        virtual auto MainLoop() -> void = 0;

        // allow the derived class to update the SCM with new status information
        auto SetStatus(unsigned long status, unsigned long error = 0) -> void;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
