/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifndef _SYNC_H
#define _SYNC_H

#include "types.h"
#include "include/ggponet.h"
#include "game_input.h"
#include "input_queue.h"
#include "ring_buffer.h"
#include "network/udp_msg.h"

#define MAX_PREDICTION_FRAMES    8

class SyncTestBackend;

class Sync {
public:
   struct Config {
      GGPOSessionCallbacks    callbacks;
      int                     num_prediction_frames;
      int                     num_players;
      int                     input_size;
   };
   struct Event {
      enum {
         ConfirmedInput,
      } type;
      union {
         struct {
            GameInput   input;
         } confirmedInput;
      } u;
   };

public:
   explicit Sync(UdpMsg::connect_status *connect_status);
   virtual ~Sync();

   void Init(Config &config);

   void SetLastConfirmedFrame(int frame);
   void SetFrameDelay(int queue, int delay) const;
   bool AddLocalInput(int queue, GameInput &input);
   void AddRemoteInput(int queue, GameInput &input) const;
   int GetConfirmedInputs(void *values, int size, int frame) const;
   int SynchronizeInputs(void *values, int size) const;

   void CheckSimulation(int timeout);
   void AdjustSimulation(int seek_to);
   void IncrementFrame(void);

   int GetFrameCount() const { return _framecount; }
   bool InRollback() const { return _rollingback; }

   bool GetEvent(Event &e);

protected:
   friend SyncTestBackend;

   struct SavedFrame {
      byte    *buf;
      int      cbuf;
      int      frame;
      int      checksum;
      SavedFrame() : buf(NULL), cbuf(0), frame(-1), checksum(0) { }
   };
   struct SavedState {
      SavedFrame frames[MAX_PREDICTION_FRAMES + 2];
      int head;
   };

   void LoadFrame(int frame);
   void SaveCurrentFrame();
   int FindSavedFrameIndex(int frame) const;
   SavedFrame &GetLastSavedFrame();

   bool CreateQueues(Config &config);
   bool CheckSimulationConsistency(int *seekTo) const;
   void ResetPrediction(int frameNumber) const;

protected:
   GGPOSessionCallbacks _callbacks;
   SavedState     _savedstate;
   Config         _config;

   bool           _rollingback;
   int            _last_confirmed_frame;
   int            _framecount;
   int            _max_prediction_frames;

   InputQueue     *_input_queues;

   RingBuffer<Event, 32> _event_queue;
   UdpMsg::connect_status *_local_connect_status;
};

#endif

