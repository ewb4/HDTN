#include "ReleaseSender.h"
#include "Uri.h"

const std::string ReleaseSender::DEFAULT_FILE = "releaseMessages1.json";


ReleaseSender::ReleaseSender() {
    m_timersFinished = false;
}

ReleaseSender::~ReleaseSender() {

}

void ReleaseSender::ProcessEvent(const boost::system::error_code&, const cbhe_eid_t finalDestinationEid, std::string message, zmq::socket_t * ptrSocket) {
  boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
  std::cout <<  "Expiry time: " << timeLocal << " , finalDestinationEid: (" << finalDestinationEid.nodeId << "," << finalDestinationEid.serviceId << ") , message: " << message;
  if (message == "start") {
      hdtn::IreleaseStartHdr releaseMsg;
      memset(&releaseMsg, 0, sizeof(hdtn::IreleaseStartHdr));
      releaseMsg.base.type = HDTN_MSGTYPE_ILINKUP;
      releaseMsg.finalDestinationEid = finalDestinationEid;
      releaseMsg.rate = 0;  //not implemented
      releaseMsg.duration = 20;  //not implemented
      ptrSocket->send(zmq::const_buffer(&releaseMsg, sizeof(hdtn::IreleaseStartHdr)), zmq::send_flags::none);
      std::cout << " -- Start Release message sent.";
  }
  else if (message == "stop") {
      hdtn::IreleaseStopHdr stopMsg;
      memset(&stopMsg, 0, sizeof(hdtn::IreleaseStopHdr));
      stopMsg.base.type = HDTN_MSGTYPE_ILINKDOWN;
      stopMsg.finalDestinationEid = finalDestinationEid;
      ptrSocket->send(zmq::const_buffer(&stopMsg, sizeof(hdtn::IreleaseStopHdr)), zmq::send_flags::none);
      std::cout << " -- Stop Release message sent.";
  }
  std::cout << std::endl << std::flush;
}

int ReleaseSender::ProcessEventFile(std::string jsonEventFileName) {
    m_timersFinished = false;
    ReleaseMessageEventVector_t releaseMessageEventVector;
    boost::property_tree::ptree pt = JsonSerializable::GetPropertyTreeFromJsonFile(jsonEventFileName);
    const boost::property_tree::ptree & releaseMessageEventsPt
            = pt.get_child("releaseMessageEvents", boost::property_tree::ptree());
    releaseMessageEventVector.resize(releaseMessageEventsPt.size());
    unsigned int eventIndex = 0;
    BOOST_FOREACH(const boost::property_tree::ptree::value_type & eventPt, releaseMessageEventsPt) {
        ReleaseMessageEvent_t & releaseMessageEvent = releaseMessageEventVector[eventIndex++];
        releaseMessageEvent.message = eventPt.second.get<std::string>("message", "");
        const std::string uriEid = eventPt.second.get<std::string>("finalDestinationEid", "");
        if (!Uri::ParseIpnUriString(uriEid, releaseMessageEvent.finalDestEid.nodeId, releaseMessageEvent.finalDestEid.serviceId)) {
            std::cerr << "error: bad uri string: " << uriEid << std::endl;
            return false;
        }
        releaseMessageEvent.delay = eventPt.second.get<int>("delay",0);
        std::string errorMessage = "";
        if ( (releaseMessageEvent.message != "start") && (releaseMessageEvent.message != "stop") ) {
            errorMessage += " Invalid message: " + releaseMessageEvent.message + ".";
        }
        if ( releaseMessageEvent.delay < 0 ) {
            errorMessage += " Invalid delay: " + boost::lexical_cast<std::string>(releaseMessageEvent.delay) + ".";
        }
        if (errorMessage.length() > 0) {
            std::cerr << errorMessage << std::endl << std::flush;
            return 1;
        }
    }

    boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
    std::cout << "Epoch Time:  " << timeLocal << std::endl << std::flush;

    zmq::context_t ctx;
    zmq::socket_t socket(ctx, zmq::socket_type::pub);
    const std::string bind_boundSchedulerPubSubPath(
        std::string("tcp://*:") + boost::lexical_cast<std::string>(m_hdtnConfig.m_zmqBoundSchedulerPubSubPortPath));
    socket.bind(bind_boundSchedulerPubSubPath);

    boost::asio::io_service ioService;
    std::vector<SmartDeadlineTimer> vectorTimers;
    vectorTimers.reserve(releaseMessageEventVector.size());
    for(std::size_t i=0; i<releaseMessageEventVector.size(); ++i) {
        SmartDeadlineTimer dt = boost::make_unique<boost::asio::deadline_timer>(ioService);
        dt->expires_from_now(boost::posix_time::seconds(releaseMessageEventVector[i].delay));
        dt->async_wait(boost::bind(&ReleaseSender::ProcessEvent,this,boost::asio::placeholders::error, releaseMessageEventVector[i].finalDestEid,
                                   releaseMessageEventVector[i].message,&socket));
        vectorTimers.push_back(std::move(dt));
    }
    ioService.run();

    socket.close();
    m_timersFinished = true;
    timeLocal = boost::posix_time::second_clock::local_time();
    std::cout << "End of ProcessEventFile:  " << timeLocal << std::endl << std::flush;
    return 0;
}

int ReleaseSender::ProcessComandLine(int argc, const char *argv[], std::string& jsonEventFileName) {
    jsonEventFileName = "";
    std::string eventsFile = ReleaseSender::DEFAULT_FILE;
    boost::program_options::options_description desc("Allowed options");
    try {
        desc.add_options()
            ("help", "Produce help message.")
            ("hdtn-config-file", boost::program_options::value<std::string>()->default_value("hdtn.json"), "HDTN Configuration File.")
            ("events-file", boost::program_options::value<std::string>()->default_value(ReleaseSender::DEFAULT_FILE),
             "Name of events file.");
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc,
                boost::program_options::command_line_style::unix_style
               | boost::program_options::command_line_style::case_insensitive), vm);
        boost::program_options::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        eventsFile = vm["events-file"].as<std::string>();
        if (eventsFile.length() < 1) {
            std::cout << desc << "\n";
            return 1;
        }
        const std::string configFileName = vm["hdtn-config-file"].as<std::string>();

        if(HdtnConfig_ptr ptrConfig = HdtnConfig::CreateFromJsonFile(configFileName)) {
            m_hdtnConfig = *ptrConfig;
        }
        else {
            std::cerr << "error loading config file: " << configFileName << std::endl;
            return false;
        }
    }
    catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "Exception of unknown type!\n";
        return 1;
    }
    std::string jsonFileName =  ReleaseSender::GetFullyQualifiedFilename(eventsFile);
    if ( !boost::filesystem::exists( jsonFileName ) ) {
        std::cerr << "File not found: " << jsonFileName << std::endl << std::flush;
        return 1;
    }
    jsonEventFileName = jsonFileName;
    return 0;
}



