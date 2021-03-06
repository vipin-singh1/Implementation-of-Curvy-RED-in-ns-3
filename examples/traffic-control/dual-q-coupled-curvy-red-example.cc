/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Srikant Singh <rathoresrikant@gmail.com>
 *          Vipin Singh <vipin.singh289@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
*/

/** Network topology
 *
 *    10Mb/s, 0ms                            10Mb/s, 0ms
 * n0--------------|                    |---------------n4
 *                 |    1.5Mbps, 20ms   |
 *                 n2------------------n3
 *    10Mb/s, 0ms  |  QueueLimit = 100  |    10Mb/s, 0ms
 * n1--------------|                    |---------------n5
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DualQCoupledCurvyRedExample");

uint32_t checkTimes;
double avgQueueDiscSize;

// The times
double global_start_time;
double global_stop_time;
double sink_start_time;
double sink_stop_time;
double client_start_time;
double client_stop_time;

NodeContainer n0n2;
NodeContainer n1n2;
NodeContainer n2n3;
NodeContainer n3n4;
NodeContainer n3n5;

Ipv4InterfaceContainer i0i2;
Ipv4InterfaceContainer i1i2;
Ipv4InterfaceContainer i2i3;
Ipv4InterfaceContainer i3i4;
Ipv4InterfaceContainer i3i5;

std::stringstream filePlotQueueDisc;
std::stringstream filePlotQueueDiscAvg;

void
CheckQueueDiscSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = StaticCast<DualQCoupledCurvyRedQueueDisc> (queue)->GetQueueSize ();

  avgQueueDiscSize += qSize;
  checkTimes++;

  // check queue disc size every 1/100 of a second
  Simulator::Schedule (Seconds (0.01), &CheckQueueDiscSize, queue);

  std::ofstream fPlotQueueDisc (filePlotQueueDisc.str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueueDisc << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueueDisc.close ();

  std::ofstream fPlotQueueDiscAvg (filePlotQueueDiscAvg.str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueueDiscAvg << Simulator::Now ().GetSeconds () << " " << avgQueueDiscSize / checkTimes << std::endl;
  fPlotQueueDiscAvg.close ();
}

void
BuildAppsTest ()
{
  // SINK1 is in the right side
  uint16_t port1 = 50000;
  Address sinkLocalAddress1 (InetSocketAddress (Ipv4Address::GetAny (), port1));
  PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", sinkLocalAddress1);
  ApplicationContainer sinkApp1 = sinkHelper1.Install (n3n4.Get (1));
  sinkApp1.Start (Seconds (sink_start_time));
  sinkApp1.Stop (Seconds (sink_stop_time));

  // SINK2 is in the right side
  uint16_t port2 = 50001;
  Address sinkLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
  PacketSinkHelper sinkHelper2 ("ns3::TcpSocketFactory", sinkLocalAddress2);
  ApplicationContainer sinkApp2 = sinkHelper2.Install (n3n5.Get (1));
  sinkApp2.Start (Seconds (sink_start_time));
  sinkApp2.Stop (Seconds (sink_stop_time));

  // Configure Classic traffic from Node 0 to Node 5
  // Configure L4S traffic from Node 1 to Node 4
  Config::Set ("/NodeList/0/$ns3::TcpL4Protocol/SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::Set ("/NodeList/1/$ns3::TcpL4Protocol/SocketType", TypeIdValue (TcpDctcp::GetTypeId ()));
  Config::Set ("/NodeList/4/$ns3::TcpL4Protocol/SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  Config::Set ("/NodeList/5/$ns3::TcpL4Protocol/SocketType", TypeIdValue (TcpDctcp::GetTypeId ()));


  // Clients are in left side
  /*
   * Create the OnOff applications to send TCP to the server
   * onoffhelper is a client that sends data to TCP destination
   */

  // Connection one
  OnOffHelper clientHelper1 ("ns3::TcpSocketFactory", Address ());
  clientHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper1.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));

  // Connection two
  OnOffHelper clientHelper2 ("ns3::TcpSocketFactory", Address ());
  clientHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  clientHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  clientHelper2.SetAttribute ("PacketSize", UintegerValue (1000));
  clientHelper2.SetAttribute ("DataRate", DataRateValue (DataRate ("10Mb/s")));

  ApplicationContainer clientApps1;
  AddressValue remoteAddress1 (InetSocketAddress (i3i4.GetAddress (1), port1));
  clientHelper1.SetAttribute ("Remote", remoteAddress1);
  clientApps1.Add (clientHelper1.Install (n0n2.Get (0)));
  clientApps1.Start (Seconds (client_start_time));
  clientApps1.Stop (Seconds (client_stop_time));

  ApplicationContainer clientApps2;
  AddressValue remoteAddress2 (InetSocketAddress (i3i5.GetAddress (1), port2));
  clientHelper2.SetAttribute ("Remote", remoteAddress2);
  clientApps2.Add (clientHelper2.Install (n1n2.Get (0)));
  clientApps2.Start (Seconds (client_start_time));
  clientApps2.Stop (Seconds (client_stop_time));
}

int
main (int argc, char *argv[])
{
  LogComponentEnable ("DualQCoupledCurvyRedQueueDisc", LOG_LEVEL_INFO);

  std::string dualQCoupledCurvyRedLinkDataRate = "1.5Mbps";
  std::string dualQCoupledCurvyRedLinkDelay = "20ms";

  std::string pathOut;
  bool writeForPlot = false;
  bool writePcap = false;
  bool flowMonitor = false;

  bool printDualQCoupledCurvyRedStats = true;

  global_start_time = 0.0;
  sink_start_time = global_start_time;
  client_start_time = global_start_time + 1.5;
  global_stop_time = 20.0;
  sink_stop_time = global_stop_time + 3.0;
  client_stop_time = global_stop_time - 2.0;

  // Configuration and command line parameter parsing
  // Will only save in the directory if enable opts below
  pathOut = "."; // Current directory
  CommandLine cmd;
  cmd.AddValue ("pathOut", "Path to save results from --writeForPlot/--writePcap/--writeFlowMonitor", pathOut);
  cmd.AddValue ("writeForPlot", "<0/1> to write results for plot (gnuplot)", writeForPlot);
  cmd.AddValue ("writePcap", "<0/1> to write results in pcapfile", writePcap);
  cmd.AddValue ("writeFlowMonitor", "<0/1> to enable Flow Monitor and write their results", flowMonitor);

  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes");
  NodeContainer c;
  c.Create (6);
  Names::Add ( "N0", c.Get (0));
  Names::Add ( "N1", c.Get (1));
  Names::Add ( "N2", c.Get (2));
  Names::Add ( "N3", c.Get (3));
  Names::Add ( "N4", c.Get (4));
  Names::Add ( "N5", c.Get (5));
  n0n2 = NodeContainer (c.Get (0), c.Get (2));
  n1n2 = NodeContainer (c.Get (1), c.Get (2));
  n2n3 = NodeContainer (c.Get (2), c.Get (3));
  n3n4 = NodeContainer (c.Get (3), c.Get (4));
  n3n5 = NodeContainer (c.Get (3), c.Get (5));

  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
  Config::SetDefault ("ns3::TcpSocketBase::UseEcn", BooleanValue (true));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (false));


  // DualQCoupledCurvyRed parameters
  NS_LOG_INFO ("Set DualQCoupledCurvyRed params");
  Config::SetDefault ("ns3::DualQCoupledCurvyRedQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_PACKETS"));
  Config::SetDefault ("ns3::DualQCoupledCurvyRedQueueDisc::L4SQueueSizeThreshold", UintegerValue (5*1500));
  Config::SetDefault ("ns3::DualQCoupledCurvyRedQueueDisc::QueueLimit", UintegerValue (100));

  NS_LOG_INFO ("Install internet stack on all nodes.");
  InternetStackHelper internet;
  internet.Install (c);

  TrafficControlHelper tchPfifo;
  uint16_t handle = tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");
  tchPfifo.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

  TrafficControlHelper tchDualQCoupledCurvyRed;
  handle = tchDualQCoupledCurvyRed.SetRootQueueDisc ("ns3::DualQCoupledCurvyRedQueueDisc");
  tchDualQCoupledCurvyRed.AddInternalQueues (handle, 2, "ns3::DropTailQueue", "MaxPackets", UintegerValue (1000));

  NS_LOG_INFO ("Create channels");
  PointToPointHelper p2p;

  NetDeviceContainer devn0n2;
  NetDeviceContainer devn1n2;
  NetDeviceContainer devn2n3;
  NetDeviceContainer devn3n4;
  NetDeviceContainer devn3n5;

  QueueDiscContainer queueDiscs;

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  devn0n2 = p2p.Install (n0n2);
  tchPfifo.Install (devn0n2);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  devn1n2 = p2p.Install (n1n2);
  tchPfifo.Install (devn1n2);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue (dualQCoupledCurvyRedLinkDataRate));
  p2p.SetChannelAttribute ("Delay", StringValue (dualQCoupledCurvyRedLinkDelay));
  devn2n3 = p2p.Install (n2n3);
  // only backbone link has DualQCoupledCurvyRed queue disc
  queueDiscs = tchDualQCoupledCurvyRed.Install (devn2n3);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  devn3n4 = p2p.Install (n3n4);
  tchPfifo.Install (devn3n4);

  p2p.SetQueue ("ns3::DropTailQueue");
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  devn3n5 = p2p.Install (n3n5);
  tchPfifo.Install (devn3n5);

  NS_LOG_INFO ("Assign IP Addresses");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  i0i2 = ipv4.Assign (devn0n2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  i1i2 = ipv4.Assign (devn1n2);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  i2i3 = ipv4.Assign (devn2n3);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  i3i4 = ipv4.Assign (devn3n4);

  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  i3i5 = ipv4.Assign (devn3n5);

  // Set up the routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  BuildAppsTest ();

  if (writePcap)
    {
      PointToPointHelper ptp;
      std::stringstream stmp;
      stmp << pathOut << "/dual-q-coupled-curvy-red";
      ptp.EnablePcapAll (stmp.str ().c_str ());
    }

  Ptr<FlowMonitor> flowmon;
  if (flowMonitor)
    {
      FlowMonitorHelper flowmonHelper;
      flowmon = flowmonHelper.InstallAll ();
    }

  if (writeForPlot)
    {
      filePlotQueueDisc << pathOut << "/" << "dual-q-coupled-curvy-red-queue-disc.plotme";
      filePlotQueueDiscAvg << pathOut << "/" << "dual-q-coupled-curvy-red-queue-disc_avg.plotme";

      remove (filePlotQueueDisc.str ().c_str ());
      remove (filePlotQueueDiscAvg.str ().c_str ());
      Ptr<QueueDisc> queue = queueDiscs.Get (0);
      Simulator::ScheduleNow (&CheckQueueDiscSize, queue);
    }

  Simulator::Stop (Seconds (sink_stop_time));
  Simulator::Run ();

  DualQCoupledCurvyRedQueueDisc::Stats st = StaticCast<DualQCoupledCurvyRedQueueDisc> (queueDiscs.Get (0))->GetStats ();

  if (flowMonitor)
    {
      std::stringstream stmp;
      stmp << pathOut << "/dual-q-coupled-curvy-red.flowmon";

      flowmon->SerializeToXmlFile (stmp.str ().c_str (), false, false);
    }

  if (printDualQCoupledCurvyRedStats)
    {
      std::cout << "*** DualQCoupledCurvyRed stats from Node 2 queue ***" << std::endl;
      std::cout << "\t " << st.unforcedClassicDrop << " Unforced drops (Classic traffic)" << std::endl;
      std::cout << "\t " << st.unforcedL4SMark << " Unforced marks (L4S traffic)" << std::endl;
      std::cout << "\t " << st.forcedDrop << " Forced drops" << std::endl;
    }

  Simulator::Destroy ();
  return 0;
}
