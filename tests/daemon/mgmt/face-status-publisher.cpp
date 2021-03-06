/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California,
 *                      Arizona Board of Regents,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University,
 *                      Washington University in St. Louis,
 *                      Beijing Institute of Technology,
 *                      The University of Memphis
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "face-status-publisher-common.hpp"

namespace nfd {
namespace tests {

NFD_LOG_INIT("FaceStatusPublisherTest");

BOOST_FIXTURE_TEST_SUITE(MgmtFaceSatusPublisher, FaceStatusPublisherFixture)

BOOST_AUTO_TEST_CASE(EncodingDecoding)
{
  Name commandName("/localhost/nfd/faces/list");
  shared_ptr<Interest> command(make_shared<Interest>(commandName));

  // MAX_SEGMENT_SIZE == 4400, FaceStatus size with filler counters is 75
  // use 59 FaceStatuses to force a FaceStatus to span Data packets
  for (int i = 0; i < 59; i++)
    {
      shared_ptr<TestCountersFace> dummy(make_shared<TestCountersFace>());

      uint64_t filler = std::numeric_limits<uint64_t>::max() - 1;
      dummy->setCounters(filler, filler, filler, filler, filler, filler);

      m_referenceFaces.push_back(dummy);

      add(dummy);
    }

  ndn::EncodingBuffer buffer;

  m_face->onReceiveData +=
    bind(&FaceStatusPublisherFixture::decodeFaceStatusBlock, this, _1);

  m_publisher.publish();
  BOOST_REQUIRE(m_finished);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace nfd
