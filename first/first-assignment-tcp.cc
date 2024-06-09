#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "tutorial-app.h"

#include <fstream>


using namespace ns3;

std::ofstream cwndFile;

void CwndChange1 (uint32_t oldCwnd, uint32_t newCwnd)
{
  cwndFile << "1" << "\t" << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
  std::cout << "1" << "\t" << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
}

void CwndChange2 (uint32_t oldCwnd, uint32_t newCwnd)
{
  cwndFile << "2" << "\t" << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
  std::cout << "2" << "\t" << Simulator::Now().GetSeconds() << "\t" << newCwnd << std::endl;
}

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  //LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
  Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(1));
  Config::SetDefault("ns3::TcpL4Protocol::RecoveryType",
                       TypeIdValue(TypeId::LookupByName("ns3::TcpClassicRecovery")));
  // Create 4 nodes
  NodeContainer nodes;
  nodes.Create (4);

  // Create point-to-point links
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Install devices on nodes
  NetDeviceContainer devices01, devices23;
  devices01 = pointToPoint.Install (nodes.Get(0), nodes.Get(1));
  devices23 = pointToPoint.Install (nodes.Get(2), nodes.Get(3));

  // Set error model, introduce a small error rate
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
  em->SetAttribute("ErrorRate", DoubleValue(0.00001));
  devices01.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  devices23.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  // Install Internet Stack
  InternetStackHelper stack;
  stack.Install (nodes);

  // Assign IP addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces01 = address.Assign (devices01);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces23 = address.Assign (devices23);

  // Get the IP addresses of the nodes
  Ipv4Address ipNode1 = interfaces01.GetAddress (1);
  Ipv4Address ipNode3 = interfaces23.GetAddress (1);

  // Configure TCP applications on nodes 0 and 1
  uint16_t port = 9;  // Port number for the server

  //Create a TCP socket
  Ptr<Socket> TcpSocket1 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange1));

  Address sinkAddress1 (InetSocketAddress (ipNode1, port));

  Ptr<TutorialApp> app1 = CreateObject<TutorialApp> ();
  app1->Setup (TcpSocket1, sinkAddress1, 1040, 10000, DataRate ("1Mbps"));
  nodes.Get (0)->AddApplication (app1);
  app1->SetStartTime (Seconds (1.0));
  app1->SetStopTime (Seconds (10.0));

  // Create a PacketSinkApplication and install it on node 1, listening only on the specific IP and port
  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (ipNode1, port));
  ApplicationContainer sinkApps = sink.Install (nodes.Get (1));
  sinkApps.Start (Seconds (1.0));
  sinkApps.Stop (Seconds (10.0));

  // Configure TCP applications on nodes 2 and 3
  port = 10;  // Different port number for the second server

  
  // Create a TCP socket
  Ptr<Socket> TcpSocket2 = Socket::CreateSocket (nodes.Get (2), TcpSocketFactory::GetTypeId ());
  TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange2));

  Address sinkAddress2 (InetSocketAddress (ipNode3, port));

  Ptr<TutorialApp> app2 = CreateObject<TutorialApp> ();
  app2->Setup (TcpSocket2, sinkAddress2, 1040, 10000, DataRate ("1Mbps"));
  nodes.Get (2)->AddApplication (app2);
  app2->SetStartTime (Seconds (1.0));
  app2->SetStopTime (Seconds (10.0));

  // Create a PacketSinkApplication and install it on node 3, listening only on the specific IP and port
  PacketSinkHelper sink2 ("ns3::TcpSocketFactory", InetSocketAddress (ipNode3, port));
  ApplicationContainer sinkApps2 = sink2.Install (nodes.Get (3));
  sinkApps2.Start (Seconds (1.0));
  sinkApps2.Stop (Seconds (10.0));

  cwndFile.open("scratch/first/first-assignment-tcp-cwnd.txt");


  // Trace RxDrop on the two devices
  //devices01.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));
  //devices23.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback (&RxDrop));  

  // Flow monitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop (Seconds (11.0));
  Simulator::Run ();

  cwndFile.close();

  monitor->CheckForLostPackets ();

  // Specify the path to save the file
  monitor->SerializeToXmlFile("scratch/first/first-ass-tcp.xml", true, true);

  //cwnd, se buffer è sempre pieno, varianza del throughput

  Simulator::Destroy ();
  return 0;
}
