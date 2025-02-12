/***************************************************************************
 * NASA Glenn Research Center, Cleveland, OH
 * Released under the NASA Open Source Agreement (NOSA)
 * May  2021
 ****************************************************************************
 */

#include "ingress.h"
#include "ZmqStorageInterface.h"
#include "EgressAsync.h"
#include "HdtnOneProcessRunner.h"
#include "SignalHandler.h"
#include "Environment.h"


#include <fstream>
#include <iostream>
#include "Logger.h"
#include "message.hpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/make_unique.hpp>
#ifdef USE_HDTN_GUI
#include "WebsocketServer.h"
 //since boost versions below 1.76 use deprecated bind.hpp in its property_tree/json_parser/detail/parser.hpp,
 //and since BOOST_BIND_GLOBAL_PLACEHOLDERS was introduced in 1.73
 //the following fixes warning:  The practice of declaring the Bind placeholders (_1, _2, ...) in the global namespace is deprecated....
 //this fixes the warning caused by boost/property_tree/json_parser/detail/parser.hpp
#if (BOOST_VERSION < 107600) && (BOOST_VERSION >= 107300) && !defined(BOOST_BIND_GLOBAL_PLACEHOLDERS)
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <boost/property_tree/json_parser.hpp> //make sure this is the very last include
#endif


void HdtnOneProcessRunner::MonitorExitKeypressThreadFunction() {
    std::cout << "Keyboard Interrupt.. exiting\n";
    hdtn::Logger::getInstance()->logNotification("ingress", "Keyboard Interrupt");
    m_runningFromSigHandler = false; //do this first
}


HdtnOneProcessRunner::HdtnOneProcessRunner() {}
HdtnOneProcessRunner::~HdtnOneProcessRunner() {}

bool HdtnOneProcessRunner::Run(int argc, const char* const argv[], volatile bool & running, bool useSignalHandler) {
    //scope to ensure clean exit before return 0
    {
        running = true;
        m_runningFromSigHandler = true;
        SignalHandler sigHandler(boost::bind(&HdtnOneProcessRunner::MonitorExitKeypressThreadFunction, this));
        bool isCutThroughOnlyTest = false;

        HdtnConfig_ptr hdtnConfig;

        boost::program_options::options_description desc("Allowed options");
        try {
            desc.add_options()
                ("help", "Produce help message.")
                ("cut-through-only-test", "Always send to egress.  Assume all links always available.")
                ("hdtn-config-file", boost::program_options::value<std::string>()->default_value("hdtn.json"), "HDTN Configuration File.")
    	        ;

            boost::program_options::variables_map vm;
            boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc, boost::program_options::command_line_style::unix_style | boost::program_options::command_line_style::case_insensitive), vm);
            boost::program_options::notify(vm);

            if (vm.count("help")) {
                std::cout << desc << "\n";
                return false;
            }

            if (vm.count("cut-through-only-test")) {
                isCutThroughOnlyTest = true;
            }

            const std::string configFileName = vm["hdtn-config-file"].as<std::string>();

            hdtnConfig = HdtnConfig::CreateFromJsonFile(configFileName);
            if (!hdtnConfig) {
                std::cerr << "error loading config file: " << configFileName << std::endl;
                return false;
            }

        }
        catch (boost::bad_any_cast & e) {
            std::cout << "invalid data error: " << e.what() << "\n\n";
            hdtn::Logger::getInstance()->logError("ingress", "Invalid Data Error: " + std::string(e.what()));
            std::cout << desc << "\n";
            return false;
        }
        catch (std::exception& e) {
            std::cerr << "error: " << e.what() << "\n";
            hdtn::Logger::getInstance()->logError("ingress", "Error: " + std::string(e.what()));
            return false;
        }
        catch (...) {
            std::cerr << "Exception of unknown type!\n";
            hdtn::Logger::getInstance()->logError("ingress", "Exception of unknown type!");
            return false;
        }

        //The io_threads argument specifies the size of the 0MQ thread pool to handle I/O operations.
        //If your application is using only the inproc transport for messaging you may set this to zero, otherwise set it to at least one.     
        std::unique_ptr<zmq::context_t> hdtnOneProcessZmqInprocContextPtr = boost::make_unique<zmq::context_t>(0);// 0 Threads

        std::cout << "starting EgressAsync.." << std::endl;
        hdtn::Logger::getInstance()->logNotification("egress", "Starting EgressAsync");

        //create on heap with unique_ptr to prevent stack overflows
        std::unique_ptr<hdtn::HegrManagerAsync> egressPtr = boost::make_unique<hdtn::HegrManagerAsync>();
        egressPtr->Init(*hdtnConfig, hdtnOneProcessZmqInprocContextPtr.get());

        printf("Announcing presence of egress ...\n");
        hdtn::Logger::getInstance()->logNotification("egress", "Egress Present");
        

        std::cout << "starting ingress.." << std::endl;
        hdtn::Logger::getInstance()->logNotification("ingress", "Starting Ingress");
        //create on heap with unique_ptr to prevent stack overflows
        std::unique_ptr<hdtn::Ingress> ingressPtr = boost::make_unique<hdtn::Ingress>();
        ingressPtr->Init(*hdtnConfig, isCutThroughOnlyTest, hdtnOneProcessZmqInprocContextPtr.get());


        //create on heap with unique_ptr to prevent stack overflows
        std::unique_ptr<ZmqStorageInterface> storagePtr = boost::make_unique<ZmqStorageInterface>();
        std::cout << "[store] Initializing storage manager ..." << std::endl;
        hdtn::Logger::getInstance()->logNotification("storage", "[store] Initializing storage manager ...");
        if (!storagePtr->Init(*hdtnConfig, hdtnOneProcessZmqInprocContextPtr.get())) {
            return false;
        }
        

        if (useSignalHandler) {
            sigHandler.Start(false);
        }
#ifdef USE_HDTN_GUI
        const boost::filesystem::path documentRootDir = Environment::GetPathHdtnSourceRoot() / "module" / "gui"; //todo
        const std::string DOCUMENT_ROOT = documentRootDir.string();
        const std::string HTML_FILE_NAME = "web_gui.html";
        const std::string PORT_NUMBER_AS_STRING = "8086";

        const boost::filesystem::path htmlMainFilePath = documentRootDir / boost::filesystem::path(HTML_FILE_NAME);
        if (boost::filesystem::is_regular_file(htmlMainFilePath)) {
            std::cout << "found " << htmlMainFilePath.string() << std::endl;
        }
        else {
            std::cout << "Cannot find " << htmlMainFilePath.string() << " : make sure document_root is set properly in allconfig.xml" << std::endl;
            return 1;
        }
        if(hdtnConfig->m_userInterfaceOn) {
            //Launch Web GUI
            std::cout << "starting websocket server\n";
            WebsocketServer server(DOCUMENT_ROOT, PORT_NUMBER_AS_STRING);

            double lastData = 0;
            double rate = 0;
            double averageRate = 0;
            std::string json;
            boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::ptime lastTime = startTime;

            //Loop to send data to webserver while HDTN is running
            while (running && m_runningFromSigHandler && !server.RequestsExit()) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
                if (useSignalHandler) {
                sigHandler.PollOnce();
                }
                boost::posix_time::ptime newTime = boost::posix_time::microsec_clock::universal_time();
                boost::posix_time::time_duration elapsedTime = newTime - lastTime;
                lastTime=newTime;
                boost::posix_time::time_duration totalTime = newTime - startTime;            

                //access ingress, egress, and storage objects to poll data and send JSON message
                rate = (8.0 * (ingressPtr->m_bundleData - lastData)) / elapsedTime.total_microseconds();
                averageRate = (8.0 * ingressPtr->m_bundleData) / totalTime.total_microseconds();
                lastData = static_cast<double>(ingressPtr->m_bundleData);

                //Create JSON
                boost::property_tree::ptree pt;
                pt.put("bundleDataRate", rate);
                pt.put("averageRate", averageRate);
                pt.put("totalData", ingressPtr->m_bundleData/1000);
                pt.put("bundleCountEgress", ingressPtr->m_bundleCountEgress);
                pt.put("bundleCountStorage", ingressPtr->m_bundleCountStorage);
                pt.put("totalBundlesErasedFromStorage", storagePtr->GetCurrentNumberOfBundlesDeletedFromStorage());
                pt.put("totalBundlesSentToEgressFromStorage", storagePtr->m_totalBundlesSentToEgressFromStorage);
                pt.put("egressBundleCount", egressPtr->m_bundleCount);
                pt.put("egressBundleData", egressPtr->m_bundleData/1000);
                pt.put("egressMessageCount", egressPtr->m_messageCount);
                std::stringstream ss;
                boost::property_tree::json_parser::write_json(ss, pt);
                json = ss.str();
                server.SendNewTextData(json.c_str(), json.size());
            }
        }
        else {
#endif //USE_HDTN_GUI
            while (running && m_runningFromSigHandler) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(250));
                if (useSignalHandler) {
                    sigHandler.PollOnce();
                }
            }
#ifdef USE_HDTN_GUI
        }
#endif
        std::ofstream output;
//        output.open("ingress-" + currentDate);

        std::ostringstream oss;
        oss << "Elapsed, Bundle Count (M),Rate (Mbps),Bundles/sec, Bundle Data "
            "(MB)\n";
        //Possibly out of Date
        double rate = 8 * ((ingressPtr->m_bundleData / (double)(1024 * 1024)) / ingressPtr->m_elapsed);
        oss << ingressPtr->m_elapsed << "," << ingressPtr->m_bundleCount / 1000000.0f << "," << rate << ","
            << ingressPtr->m_bundleCount / ingressPtr->m_elapsed << ", " << ingressPtr->m_bundleData / (double)(1024 * 1024) << "\n";

        std::cout << oss.str();
//        output << oss.str();
//        output.close();
        hdtn::Logger::getInstance()->logInfo("ingress", oss.str());

        boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
        std::cout << "IngressAsyncRunner currentTime  " << timeLocal << std::endl << std::flush;

        std::cout << "IngressAsyncRunner: exiting cleanly..\n";
        ingressPtr->Stop();
        m_ingressBundleCountStorage = ingressPtr->m_bundleCountStorage;
        m_ingressBundleCountEgress = ingressPtr->m_bundleCountEgress;
        m_ingressBundleCount = ingressPtr->m_bundleCount;
        m_ingressBundleData = ingressPtr->m_bundleData;

        std::cout << "StorageRunner: exiting cleanly..\n";
        storagePtr->Stop();
        m_totalBundlesErasedFromStorage = storagePtr->GetCurrentNumberOfBundlesDeletedFromStorage();
        m_totalBundlesSentToEgressFromStorage = storagePtr->m_totalBundlesSentToEgressFromStorage;

        std::cout << "EgressAsyncRunner: exiting cleanly..\n";
        egressPtr->Stop();
        m_egressBundleCount = egressPtr->m_bundleCount;
        m_egressBundleData = egressPtr->m_bundleData;
        m_egressMessageCount = egressPtr->m_messageCount;
    }
    std::cout << "HDTN one process: exited cleanly\n";
    return true;
}
