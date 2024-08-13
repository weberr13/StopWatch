/* 
 * File:   AlarmClockTest.cpp
 * Author: Craig Cogdill
 * Created: October 15, 2015 10:45am
 * Modifier: Amanda Carbonari
 * Modified Date: January 5, 2016 3:45pm
 */

#include "AlarmClockTest.h"
#include "AlarmClock.h"
#include "StopWatch.h"
#include <chrono>
#include <iostream>
#include <thread>

std::atomic<unsigned int> AlarmClockTest::mFakeSleepUs(0);


namespace {
   typedef std::chrono::microseconds microseconds;
   typedef std::chrono::milliseconds milliseconds;
   typedef std::chrono::seconds seconds;

   unsigned int kFakeSleepLeeway = 100;

   template<typename T>
   void WaitForAlarmClockToExpire(AlarmClock<T>& alerter) {
      while (!alerter.Expired());
   }

   template<typename Duration>
   int ConvertToMicroSeconds(Duration t) {
      return static_cast<int>(std::chrono::duration_cast<microseconds>(t).count());
   }

   bool FakeSleep(unsigned int usToSleep) {
      AlarmClockTest::mFakeSleepUs.store(usToSleep);
      std::this_thread::sleep_for(microseconds(10));
      return true;
   }
}

TEST_F(AlarmClockTest, GetUsSleepTimeInUs) {
   int us = 123456;
   AlarmClock<microseconds> alerter(us);
   EXPECT_EQ(us, alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetMsSleepTimeInUs) {
   int ms = 123456;
   AlarmClock<milliseconds> alerter(ms);
   EXPECT_EQ(ConvertToMicroSeconds(milliseconds(ms)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, GetSecSleepTimeInUs) {
   int sec = 1;
   AlarmClock<seconds> alerter(sec);
   EXPECT_EQ(ConvertToMicroSeconds(seconds(sec)), alerter.SleepTimeUs());
}

TEST_F(AlarmClockTest, microsecondsLessThan500ms) {
   int us = 900;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(us));
}

TEST_F(AlarmClockTest, microsecondsGreaterThan500ms) {
   int us = 600000;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(us));
}

/*TEST_F(AlarmClockTest, weirdNumberOfMicroseconds) {
   int us = 724509;
   StopWatch sw;
   AlarmClock<microseconds> alerter(us, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, us-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(us));
}*/

TEST_F(AlarmClockTest, millisecondsLessThan500) {
   unsigned int ms = 100;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(alerter.SleepTimeUs()));
}

TEST_F(AlarmClockTest, oneSecondInMilliseconds) {
   unsigned int ms = 1000;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(alerter.SleepTimeUs()));
}

TEST_F(AlarmClockTest, millisecondsNotDivisibleBy500) {
   unsigned int ms = 1000;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(alerter.SleepTimeUs()));
}

TEST_F(AlarmClockTest, secondsSimple) {
   unsigned int sec = 1;
   AlarmClock<seconds> alerter(sec, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   EXPECT_GE(AlarmClockTest::mFakeSleepUs, alerter.SleepTimeUs()-kFakeSleepLeeway);
   EXPECT_LE(AlarmClockTest::mFakeSleepUs, static_cast<unsigned int>(alerter.SleepTimeUs()));
}

TEST_F(AlarmClockTest, LongTimeout_ImmediatelyDestructed) {
   unsigned int sec = 1000;
   StopWatch sw;
   std::unique_ptr<AlarmClock<seconds>> acPtr(new AlarmClock<seconds>(sec));
   EXPECT_EQ(ConvertToMicroSeconds(seconds(sec)), acPtr->SleepTimeUs());
   acPtr.reset();
   EXPECT_TRUE(sw.ElapsedSec() < 2);
}

TEST_F(AlarmClockTest, milliseconds_ResetAfterExpired) {
   // First run
   int ms = 750;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   
   // Reset after AlarmClock has expired
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}

TEST_F(AlarmClockTest, milliseconds_ResetBeforeExpired) {
   int ms = 7500;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   alerter.Reset();
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}

TEST_F(AlarmClockTest, milliseconds_MultipleResetsAfterExpired) {
   // First run
   int ms = 750;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
   
   // Reset after AlarmClock has expired
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());

   // Reset again after it has expired
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}

TEST_F(AlarmClockTest, milliseconds_MultipleResetsBeforeExpired) {
   int ms = 7500;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   alerter.Reset();
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}

TEST_F(AlarmClockTest, milliseconds_MultipleResetsMixed) {
   int ms = 750;
   AlarmClock<milliseconds> alerter(ms, FakeSleep);
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());

   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());

   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   alerter.Reset();
   EXPECT_FALSE(alerter.Expired());
   WaitForAlarmClockToExpire(alerter);
   EXPECT_TRUE(alerter.Expired());
}
