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

#include "ns3/test.h"
#include "ns3/dual-q-coupled-curvy-red-queue-disc.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

class DualQueueL4SQueueDiscTestItem1 : public QueueDiscItem
{
public:
  DualQueueL4SQueueDiscTestItem1 (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~DualQueueL4SQueueDiscTestItem1 ();
  virtual void AddHeader (void);
  virtual bool Mark (void);
  virtual bool IsL4S (void);

private:
  DualQueueL4SQueueDiscTestItem1 ();
  DualQueueL4SQueueDiscTestItem1 (const DualQueueL4SQueueDiscTestItem1 &);
  DualQueueL4SQueueDiscTestItem1 &operator = (const DualQueueL4SQueueDiscTestItem1 &);
};

DualQueueL4SQueueDiscTestItem1::DualQueueL4SQueueDiscTestItem1 (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

DualQueueL4SQueueDiscTestItem1::~DualQueueL4SQueueDiscTestItem1 ()
{
}

void
DualQueueL4SQueueDiscTestItem1::AddHeader (void)
{
}

bool
DualQueueL4SQueueDiscTestItem1::Mark (void)
{
  return true;
}

bool
DualQueueL4SQueueDiscTestItem1::IsL4S (void)
{
  return true;
}

class DualQueueClassicQueueDiscTestItem1 : public QueueDiscItem
{
public:
  DualQueueClassicQueueDiscTestItem1 (Ptr<Packet> p, const Address & addr, uint16_t protocol);
  virtual ~DualQueueClassicQueueDiscTestItem1 ();
  virtual void AddHeader (void);
  virtual bool Mark (void);
  virtual bool IsL4S (void);

private:
  DualQueueClassicQueueDiscTestItem1 ();
  DualQueueClassicQueueDiscTestItem1 (const DualQueueClassicQueueDiscTestItem1 &);
  DualQueueClassicQueueDiscTestItem1 &operator = (const DualQueueClassicQueueDiscTestItem1 &);
};

DualQueueClassicQueueDiscTestItem1::DualQueueClassicQueueDiscTestItem1 (Ptr<Packet> p, const Address & addr, uint16_t protocol)
  : QueueDiscItem (p, addr, protocol)
{
}

DualQueueClassicQueueDiscTestItem1::~DualQueueClassicQueueDiscTestItem1 ()
{
}

void
DualQueueClassicQueueDiscTestItem1::AddHeader (void)
{
}

bool
DualQueueClassicQueueDiscTestItem1::Mark (void)
{
  return true;
}

bool
DualQueueClassicQueueDiscTestItem1::IsL4S (void)
{
  return false;
}

class DualQCoupledCurvyRedQueueDiscTestCase : public TestCase
{
public:
  DualQCoupledCurvyRedQueueDiscTestCase ();
  virtual void DoRun (void);
private:
  void Enqueue (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void EnqueueWithDelay (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType);
  void Dequeue (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t nPkt);
  void DequeueWithDelay (Ptr<DualQCoupledCurvyRedQueueDisc> queue, double delay, uint32_t nPkt);
  void RunCurvyRedTest (StringValue mode);
};

DualQCoupledCurvyRedQueueDiscTestCase::DualQCoupledCurvyRedQueueDiscTestCase ()
  : TestCase ("Sanity check on the DualQ Coupled Curvy Red queue disc implementation")
{
}

void
DualQCoupledCurvyRedQueueDiscTestCase::RunCurvyRedTest (StringValue mode)
{
  uint32_t pktSize = 0;

  // 1 for packets; pktSize for bytes
  uint32_t modeSize = 1;

  uint32_t qSize = 50;
  Ptr<DualQCoupledCurvyRedQueueDisc> queue = CreateObject<DualQCoupledCurvyRedQueueDisc> ();


  // test 1: simple enqueue/dequeue with defaults, no drops
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");

  Address dest;

  if (queue->GetMode () == DualQCoupledCurvyRedQueueDisc::QUEUE_DISC_MODE_BYTES)
    {
      // pktSize should be same as MeanPktSize to avoid performance gap between byte and packet mode
      pktSize = 1000;
      modeSize = pktSize;
      qSize = qSize * modeSize;
    }

  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");

  Ptr<Packet> p1, p2, p3, p4, p5, p6, p7, p8;
  p1 = Create<Packet> (pktSize);
  p2 = Create<Packet> (pktSize);
  p3 = Create<Packet> (pktSize);
  p4 = Create<Packet> (pktSize);
  p5 = Create<Packet> (pktSize);
  p6 = Create<Packet> (pktSize);
  p7 = Create<Packet> (pktSize);
  p8 = Create<Packet> (pktSize);

  queue->Initialize ();
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 0 * modeSize, "There should be no packets in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem1> (p1, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 1 * modeSize, "There should be one packet in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem1> (p2, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 2 * modeSize, "There should be two packets in there");
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem1> (p3, dest, 0));
  queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem1> (p4, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem1> (p5, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem1> (p6, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem1> (p7, dest, 0));
  queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem1> (p8, dest, 0));
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 8 * modeSize, "There should be eight packets in there");

  Ptr<QueueDiscItem> item;

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the first packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 7 * modeSize, "There should be seven packets in there");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the second packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 6 * modeSize, "There should be six packet in there");

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item != 0), true, "I want to remove the third packet");
  NS_TEST_EXPECT_MSG_EQ (queue->GetQueueSize (), 5 * modeSize, "There should be five packets in there");

  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();
  item = queue->Dequeue ();

  item = queue->Dequeue ();
  NS_TEST_EXPECT_MSG_EQ ((item == 0), true, "There are really no packets in there");
  
  // test 2: Test by sending both L4S traffic and classic traffic
  queue = CreateObject<DualQCoupledCurvyRedQueueDisc> ();
  pktSize = 1000;  // pktSize != 0 because DequeueThreshold always works in bytes
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K0", UintegerValue (1)), true,
                         "Verify that we can actually set the attribute K0");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueScalingFactor", DoubleValue (-1)), true,
                         "Verify that we can actually set the attribute ClassicQueueScalingFactor");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Curviness", UintegerValue(1)), true,
                         "Verify that we can actually set the attribute Curviness");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SQueueSizeThreshold", UintegerValue (5*1500)), true,
                         "Verify that we can actually set the attribute L4SQueueSizeThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Fc", UintegerValue (5)), true,
                         "Verify that we can actually set the attribute Fc");

  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 200, StringValue ("L4S"));
  EnqueueWithDelay (queue, pktSize, 200, StringValue ("Classic"));
  DequeueWithDelay (queue, 0.012, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  DualQCoupledCurvyRedQueueDisc::Stats st = StaticCast<DualQCoupledCurvyRedQueueDisc> (queue)->GetStats ();
  uint32_t test2L4SMark = st.unforcedL4SMark;
  NS_TEST_EXPECT_MSG_NE (test2L4SMark, 0, "There should some unforced l4s marks");
  NS_TEST_EXPECT_MSG_NE (st.unforcedClassicDrop, 0, "There should some unforced classic drops");
  NS_TEST_EXPECT_MSG_NE (st.forcedDrop, 0, "There should be some forced drops");

  // test 3: Test by sending L4S traffic only
  queue = CreateObject<DualQCoupledCurvyRedQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K0", UintegerValue (1)), true,
                         "Verify that we can actually set the attribute K0");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueScalingFactor", DoubleValue (-1)), true,
                         "Verify that we can actually set the attribute ClassicQueueScalingFactor");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Curviness", UintegerValue(1)), true,
                         "Verify that we can actually set the attribute Curviness");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SQueueSizeThreshold", UintegerValue (5*1500)), true,
                         "Verify that we can actually set the attribute L4SQueueSizeThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Fc",UintegerValue (5)), true,
                         "Verify that we can actually set the attribute Fc");

  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue ("L4S"));
  DequeueWithDelay (queue, 0.012, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<DualQCoupledCurvyRedQueueDisc> (queue)->GetStats ();
  NS_TEST_EXPECT_MSG_EQ (st.unforcedClassicDrop, 0, "There should be zero unforced classic drops since only L4S traffic is pumped ");
  NS_TEST_EXPECT_MSG_NE (st.unforcedL4SMark, 0, "There should be some L4S marks");
 
  // test 4: Test by sending Classic traffic only
  queue = CreateObject<DualQCoupledCurvyRedQueueDisc> ();
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Mode", mode), true,
                         "Verify that we can actually set the attribute Mode");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("QueueLimit", UintegerValue (qSize)), true,
                         "Verify that we can actually set the attribute QueueLimit");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("K0", UintegerValue (1)), true,
                         "Verify that we can actually set the attribute K0");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("ClassicQueueScalingFactor", DoubleValue (-1)), true,
                         "Verify that we can actually set the attribute ClassicQueueScalingFactor");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Curviness", UintegerValue(1)), true,
                         "Verify that we can actually set the attribute Curviness");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("L4SQueueSizeThreshold", UintegerValue (5*1500)), true,
                         "Verify that we can actually set the attribute L4SQueueSizeThreshold");
  NS_TEST_EXPECT_MSG_EQ (queue->SetAttributeFailSafe ("Fc", UintegerValue (5)), true,
                         "Verify that we can actually set the attribute Fc");

  queue->Initialize ();
  EnqueueWithDelay (queue, pktSize, 400, StringValue ("Classic"));
  DequeueWithDelay (queue, 0.012, 400);
  Simulator::Stop (Seconds (8.0));
  Simulator::Run ();
  st = StaticCast<DualQCoupledCurvyRedQueueDisc> (queue)->GetStats ();
  NS_TEST_EXPECT_MSG_NE (st.unforcedClassicDrop, 0, "There should be some unforced classic drops since we are not marking classic packets ");
  NS_TEST_EXPECT_MSG_EQ (st.unforcedL4SMark, 0, "There should be zero L4S marks since only Classic traffic is pumped"); 
}

void
DualQCoupledCurvyRedQueueDiscTestCase::Enqueue (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  for (uint32_t i = 0; i < nPkt; i++)
    {
      if (trafficType.Get () == "L4S")
        {
          queue->Enqueue (Create<DualQueueL4SQueueDiscTestItem1> (Create<Packet> (size), dest, 0));
        }
      else if (trafficType.Get () == "Classic")
        {
          queue->Enqueue (Create<DualQueueClassicQueueDiscTestItem1> (Create<Packet> (size), dest, 0));
        }
    }
}

void
DualQCoupledCurvyRedQueueDiscTestCase::EnqueueWithDelay (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t size, uint32_t nPkt, StringValue trafficType)
{
  Address dest;
  double delay = 0.01;  // enqueue packets with delay
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds (i * delay)), &DualQCoupledCurvyRedQueueDiscTestCase::Enqueue, this, queue, size, 1, trafficType);
    }
}

void
DualQCoupledCurvyRedQueueDiscTestCase::Dequeue (Ptr<DualQCoupledCurvyRedQueueDisc> queue, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Ptr<QueueDiscItem> item = queue->Dequeue ();
    }
}

void
DualQCoupledCurvyRedQueueDiscTestCase::DequeueWithDelay (Ptr<DualQCoupledCurvyRedQueueDisc> queue, double delay, uint32_t nPkt)
{
  for (uint32_t i = 0; i < nPkt; i++)
    {
      Simulator::Schedule (Time (Seconds ((i + 1) * delay)), &DualQCoupledCurvyRedQueueDiscTestCase::Dequeue, this, queue, 1);
    }
}

void
DualQCoupledCurvyRedQueueDiscTestCase::DoRun (void)
{
  RunCurvyRedTest (StringValue ("QUEUE_DISC_MODE_PACKETS"));
  RunCurvyRedTest (StringValue ("QUEUE_DISC_MODE_BYTES"));
  Simulator::Destroy ();
}

static class DualQCoupledCurvyRedQueueDiscTestSuite : public TestSuite
{
public:
  DualQCoupledCurvyRedQueueDiscTestSuite ()
    : TestSuite ("dual-q-coupled-curvy-red-queue-disc", UNIT)
  {
    AddTestCase (new DualQCoupledCurvyRedQueueDiscTestCase (), TestCase::QUICK);
  }
} g_DualQCoupledCurvyRedQueueTestSuite;
