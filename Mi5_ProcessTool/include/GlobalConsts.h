#ifndef GLOBALCONSTS_H
#define GLOBALCONSTS_H

// Module numbers
static const int MODULENUMBERXTS1 = 2001; //2001
static const int MODULENUMBERXTS2 = 2002;
static const int MODULENUMBERXTS3 = 2003;
static const int MODULENUMBERCOOKIESEPARATOR = 2101;

// Maintenance Error IDs
static const int MODULECOOKIEREFILLERRORID = 123;

// Manual modules
static const int MANUALMODULE1 = 2403;
static const int MAINTENANCEMODULE = 2402;

// Simu modules
static const int MODULEX = 1202;
static const int MODULEY = 1203;
static const int MODULEZ = 1204;

static const int MODULENUMBERCOOKIE = 2101;
static const int MODULENUMBERCREME = 2;
static const int MODULENUMBERTASK = 11;
static const int MODULENUMBERXTSMIN = 2001;  //2001
static const int MODULENUMBERXTSMAX = 2005; //2005
static const int MODULENUMBERMESSAGEFEEDER = 10;

// Specific skills
static const int MANUALUNIVERSALSKILL = 1400;
static const int POSCALSKILLID = 1321;
static const int SKILLTRANSPORT = 1320; // Param1: pos x
static const int SKILLIDXTSRESERVE = 1310;
static const int SKILLIDXTSRELEASE = 1311;
static const int SKILLIDXTSBLOCK = 1315;
static const int SKILLIDXTSUNBLOCK = 1316;

enum messageFeedLevel
{
    msgClear = 0,
    msgSuccess = 1,
    msgInfo,
    msgWarning,
    msgError,
    msgFatal
};

#endif // GLOBALCONSTS_H