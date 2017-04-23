#ifndef VRPN_TRACKER_FILE_H
#define VRPN_TRACKER_FILE_H

#include "vrpn_Configure.h"  //  VRPN_API
#include "vrpn_Tracker.h"    //  vrpn_Tracker
#include "vrpn_Types.h"      // vrpn_ etc

class VRPN_API vrpn_Connection;

class VRPN_API vrpn_Tracker_File: public vrpn_Tracker
{
	public:
		vrpn_Tracker_File(const char * name, vrpn_Connection * c, 
			vrpn_float64 rate, const char * filename);
		~vrpn_Tracker_File();
		virtual void mainloop();

	private:
		vrpn_float64 update_rate;
		void * filehandle;
};

#endif // VRPN_TRACKER_FILE_H