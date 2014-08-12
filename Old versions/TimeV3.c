#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1, LL_pot, sensorPotentiometer)
#pragma config(Sensor, in2, RL_pot, sensorPotentiometer)
#pragma config(Sensor, in3, One, sensorPotentiometer)
#pragma config(Sensor, I2C_1, RDB_encoder, sensorQuadEncoderOnI2CPort, , AutoAssign)
#pragma config(Sensor, I2C_2, RDF_encoder, sensorQuadEncoderOnI2CPort, , AutoAssign)
#pragma config(Sensor, I2C_3, LDF_encoder, sensorQuadEncoderOnI2CPort, , AutoAssign)
#pragma config(Sensor, I2C_4, LDB_encoder, sensorQuadEncoderOnI2CPort, , AutoAssign)
#pragma config(Motor, port1, RDF, tmotorVex393, openLoop, encoder, encoderPort, I2C_2, 1000)
#pragma config(Motor, port2, RDB, tmotorVex393, openLoop, encoder, encoderPort, I2C_1, 1000)
#pragma config(Motor, port3, LLU, tmotorVex393, openLoop, reversed)
#pragma config(Motor, port4, LLD, tmotorVex393, openLoop)
#pragma config(Motor, port5, RLU, tmotorVex393, openLoop)
#pragma config(Motor, port6, RLD, tmotorVex393, openLoop, reversed)
#pragma config(Motor, port7, RIN, tmotorVex393, openLoop)
#pragma config(Motor, port8, LIN, tmotorVex393, openLoop)
#pragma config(Motor, port9, LDB, tmotorVex393, openLoop, reversed, encoder, encoderPort, I2C_4, 1000)
#pragma config(Motor, port10, LDF, stmotorVex393, openLoop, encoder, encoderPort, I2C_3, 1000)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

//Definitions
#define round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))

//Global Variables
bool speedstepDriveEnabled = true;
bool speedstepLiftEnabled = false;
bool expCtrlEnabled = true;
bool liftPresetEnabled = true;
bool interrupt = false;
bool mountedSkyrise;
int liftPreset = 0;
const int expScalingVal = 5;
const int liftVal[] = {1, 1550, 680, 1140, 1400, 1500};
int liftAutonVal[] = {0, 0};
int liftError = SensorValue[One] - 40;

//const int limit_LL_pot = 1460;
//const int limit_RL_pot = 1592;
//const float pot_scaling = (float) (limit_LL_pot/limit_RL_pot);
const int trimSwitch = 150;

//Motor Usercontrol Variables
int rightLift;
int leftLift;
int leftDF;
int rightDF;
int leftDB;
int rightDB;

//Initialise speedstep variables
//	float target_scale;
	int speed = 0;
	int error = 0;
	int integral = 0;
	int derivitive = 0;
	int previousError[] = {0, 0, 0, 0};

//Tune these values
	float Kp = 0.5;
	float Ki = 0.12;
	float Kd = 0.1;


/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

//Experimental Functions
	//Controller Exponential Scaling
	int expCtrl(int input, int modifier) //ctrlIN from controller value, mod is set to driver prefrences
	{
		if (expCtrlEnabled)
		{
			int sign = input / abs(input);
			input = abs(input);
			modifier = abs(modifier);
			float scalingValue = 127.0 / (modifier - 1);
			float percentMax = input / 127.0;
			float expModifier = (float)(pow(modifier, percentMax) - 1);
			int output = (int)round(scalingValue * expModifier * sign);
			return output;
		}
		else
		{
			return input;
		}
	}

	//Speedstep for Drive
	int speedstepDrive(int target, int speedLimit, int sensorReading, int arrayId) //PID for Autonomus Drive
	{
		if (speedstepDriveEnabled)
		{
			error = target - sensorReading;
			integral = integral + error;
			if (error == 0)
			{
				integral = 0;
			}
			if (abs(error) > 40)
			{
				integral = 0;
			}
			derivitive = error - previousError[arrayId];
			previousError[arrayId] = error;
			speed = (int) round(Kp * error + Ki * integral + Kd * derivitive);
			if (speed > speedLimit)
			{
				speed = speedLimit;
			}
			return speed;
		}
		else
		{
			return speedLimit;
		}
	}
	//Speedstep for Lift
	int speedstepLift(int target, int speedLimit, int sensorReading, int arrayId) //PID for Autonomus Drive
	{
		if (speedstepLiftEnabled)
		{
			error = target - sensorReading;
			integral = integral + error;
			if (error == 0)
			{
				integral = 0;
			}
			if (abs(error) > 40)
			{
				integral = 0;
			}
			derivitive = error - previousError[arrayId];
			previousError[arrayId] = error;
			speed = Kp * error + Ki * integral + Kd * derivitive;
			if (speed > speedLimit)
			{
				speed = speedLimit;
			}
			return speed;
		}
		else
		{
			return speedLimit;
		}
	}

//Autonomous Functions

void clearEncoders()
{
	nMotorEncoder[LDF] = 0;
	nMotorEncoder[RDF] = 0;
	nMotorEncoder[LDB] = 0;
	nMotorEncoder[RDB] = 0;
}

void clearPreviousError()
{
	previousError[0] = 0;
	previousError[1] = 0;
	previousError[2] = 0;
	previousError[3] = 0;
}

void extStoppers()
{
	motor[LDB] = 127;
	motor[RDB] = 127;
	motor[LDF] =- 127;
	motor[RDF] =- 127;
	wait1Msec(1000);
	motor[LDB] = 0;
	motor[RDB] = 0;
	motor[LDF] = 0;
	motor[RDF] = 0;
}

void tMove(int time, int speed) //Goes forward, neg speed for back
{
	motor[LDB] = speed;
	motor[RDB] = speed;
	motor[LDF] = speed;
	motor[RDF] = speed;
	wait1Msec(time);
	motor[LDB] = 0;
	motor[RDB] = 0;
	motor[LDF] = 0;
	motor[RDF] = 0;
}

void tStrafe(int time, int speed) //Strafes right, neg speed for left
{
	motor[LDF] = speed;
	motor[LDB] =- speed;
	motor[RDF] =- speed;
	motor[RDB] = speed;
}

void tRTurn(int time, int speed) //Turns to right, neg speed for left
{
	motor[LDF] = speed;
	motor[LDB] = speed;
	motor[RDF] =- speed;
	motor[RDB] =- speed;
	wait1Msec(time);
	motor[LDB] = 0;
	motor[RDB] = 0;
	motor[LDF] = 0;
	motor[RDF] = 0;
}

void tLift(int time, int speed)
{
	motor[LLD] = speed;
	motor[LLU] = speed;
	motor[RLD] = speed;
	motor[RLU] = speed;
	wait1Msec(time);
	motor[LLD] = 10;
	motor[LLU] = 10;
	motor[RLD] = 10;
	motor[RLU] = 10;
}

void driveMove(int encoderDistance, int speed)
{
	clearPreviousError();
	bool LDBgoalReached = false;
	bool RDBgoalReached = false;
	bool LDFgoalReached = false;
	bool RDFgoalReached = false;
	//Data validation
	speed = abs(speed);
	while ((!LDBgoalReached) || (!RDBgoalReached) || (!LDFgoalReached) || (!RDFgoalReached))
	{
		if (abs(nMotorEncoder[LDB]) < abs(encoderDistance))
		{
			motor[LDB] = speedstepDrive(encoderDistance, speed, nMotorEncoder[LDB], 3);
		}
		else
		{
			motor[LDB] = 0;
			LDBgoalReached = true;
		}
		if (abs(nMotorEncoder[RDB]) < abs(encoderDistance))
		{
			motor[RDB] = speedstepDrive(encoderDistance, speed, nMotorEncoder[RDB], 0);
		}
		else
		{
			motor[RDB] = 0;
			RDBgoalReached = true;
		}
		if (abs(nMotorEncoder[LDF]) < abs(encoderDistance))
		{
			motor[LDF] = speedstepDrive(encoderDistance, speed, nMotorEncoder[LDF], 2);
		}
		else
		{
			motor[LDF] = 0;
			LDFgoalReached = true;
		}
		if (abs(nMotorEncoder[RDF]) < abs(encoderDistance))
		{
			motor[RDF] = speedstepDrive(encoderDistance, speed, nMotorEncoder[RDF], 1);
		}
		else
		{
			motor[RDF] = 0;
			RDFgoalReached = true;
		}
	}
}

void driveTurn(int encoderDistance, int speed)
{
	clearPreviousError();
	bool LDBgoalReached = false;
	bool RDBgoalReached = false;
	bool LDFgoalReached = false;
	bool RDFgoalReached = false;
	//Data validation
	speed = abs(speed);
	while ((!LDBgoalReached) || (!RDBgoalReached) || (!LDFgoalReached) || (!RDFgoalReached))
	{
		if (abs(nMotorEncoder[LDB]) < abs(encoderDistance))
		{
			motor[LDB] = speedstepDrive(encoderDistance, speed, nMotorEncoder[LDB], 3);
		}
		else
		{
			motor[LDB] = 0;
			LDBgoalReached = true;
		}
		if (abs(nMotorEncoder[RDB]) < abs(encoderDistance))
		{
			motor[RDB] = -(speedstepDrive(encoderDistance, speed, nMotorEncoder[RDB], 0));
		}
		else
		{
			motor[RDB] = 0;
			RDBgoalReached = true;
		}
		if (abs(nMotorEncoder[LDF]) < abs(encoderDistance))
		{
			motor[LDF] = speedstepDrive(encoderDistance, speed, LDF_encoder, 2);
		}
		else
		{
			motor[LDF] = 0;
			LDFgoalReached = true;
		}
		if (abs(nMotorEncoder[RDF]) < abs(encoderDistance))
		{
			motor[RDF] = -(speedstepDrive(encoderDistance, speed, RDF_encoder, 1));
		}
		else
		{
			motor[RDF] = 0;
			RDFgoalReached = true;
		}
	}
}

void driveStrafe(int encoderDistance, int speed)
{
	clearPreviousError();
	bool LDBgoalReached = false;
	bool RDBgoalReached = false;
	bool LDFgoalReached = false;
	bool RDFgoalReached = false;
	//Data validation
	speed = abs(speed);
	while ((!LDBgoalReached) || (!RDBgoalReached) || (!LDFgoalReached) || (!RDFgoalReached))
	{
		if (abs(nMotorEncoder[LDB]) < abs(encoderDistance))
		{
			motor[LDB] = -(speedstepDrive(encoderDistance, speed, nMotorEncoder[LDB], 3));
		}
		else
		{
			motor[LDB] = 0;
			LDBgoalReached = true;
		}
		if (abs(nMotorEncoder[RDB]) < abs(encoderDistance))
		{
			motor[RDB] = speedstepDrive(encoderDistance, speed, nMotorEncoder[RDB], 0);
		}
		else
		{
			motor[RDB] = 0;
			RDBgoalReached = true;
		}
		if (abs(nMotorEncoder[LDF]) < abs(encoderDistance))
		{
			motor[LDF] = speedstepDrive(encoderDistance, speed, LDF_encoder, 2);
		}
		else
		{
			motor[LDF] = 0;
			LDFgoalReached = true;
		}
		if (abs(nMotorEncoder[RDF]) < abs(encoderDistance))
		{
			motor[RDF] = -(speedstepDrive(encoderDistance, speed, RDF_encoder, 1));
		}
		else
		{
			motor[RDF] = 0;
			RDFgoalReached = true;
		}
	}
}

void liftMove(int target, int speed)
{
	clearPreviousError();
	liftAutonVal[0] = target;
	liftAutonVal[1] = speed;
}

void liftAutonMonitor()
{
	if (liftAutonVal[0] != 0)
	{
		bool LLgoalReached = false;
		bool RLgoalReached = false;
		string direction; // recommend a bool or an enum instead. Strings take up a significant amount of memory
		if (SensorValue[RL_pot] < liftAutonVal[0])
		{
			direction = "up";
		}
		if (SensorValue[RL_pot] > liftAutonVal[0])
		{
			direction = "down";
		}
		if ((abs(SensorValue[RL_pot]) - 30 < abs(liftAutonVal[0])) && ((abs(SensorValue[RL_pot]) + 30 > abs(liftAutonVal[0]))))
		{
			leftLift = 0;
			rightLift = 0;
			liftAutonVal[0] = 0;
			liftPreset = 0;
		}
		else if ((liftAutonVal[0] != 0))
		{
			if (direction == "down")
			{
				while ((!LLgoalReached) || (!RLgoalReached))
				{
					if (SensorValue[LL_pot] > liftAutonVal[0] - 60)
					{
						leftLift = speedstepLift(liftAutonVal[0] - 60, -(liftAutonVal[1]), SensorValue[LL_pot], 0);
						motor[LLD] = leftLift;
						motor[LLU] = leftLift;
					}
					else
					{
						LLgoalReached = true;
						leftLift = 0;
					}
					if (SensorValue[RL_pot] > liftAutonVal[0])
					{
						rightLift = speedstepLift(liftAutonVal[0], -(liftAutonVal[1]), SensorValue[RL_pot], 1);
						motor[RLD] = rightLift;
						motor[RLU] = rightLift;
					}
					else
					{
						RLgoalReached = true;
						rightLift = 0;
					}
				}
			}
			if (direction == "up")
			{
				while ((!LLgoalReached) || (!RLgoalReached))
				{
					if (SensorValue[LL_pot] < liftAutonVal[0] - 60)
					{
						leftLift = speedstepLift(liftAutonVal[0] - 60, (liftAutonVal[1]), SensorValue[LL_pot], 0);
						motor[LLD] = leftLift;
						motor[LLU] = leftLift;
					}
					else
					{
						LLgoalReached = true;
						leftLift = 0;
					}
					if (SensorValue[RL_pot] < liftAutonVal[0])
					{
						rightLift = speedstepLift(liftAutonVal[0], liftAutonVal[1], SensorValue[RL_pot], 1);
						motor[RLD] = rightLift;
						motor[RLU] = rightLift;
					}
					else
					{
						RLgoalReached = true;
						rightLift = 0;
					}
				}
			}
		}
	}
}

void intakeOn()
{
	motor[LIN] = 127;
	motor[RIN] = 127;
}
void intakeReverse()
{
	motor[LIN] = 127;
	motor[RIN] = 127;
}
void intakeOff()
{
	motor[LIN] = 0;
	motor[RIN] = 0;
}
//Driver Functions
//Preset Button Control
void presetButtons()
{
	if (liftPresetEnabled)
	{
		if (vexRT[Btn8U] == 1)
		{
			clearPreviousError();
			liftPreset = 6;
		}
		if (vexRT[Btn8L] == 1)
		{
			clearPreviousError();
			liftPreset = 5;
		}
		if (vexRT[Btn8R] == 1)
		{
			clearPreviousError();
			liftPreset= 4;
		}
		if (vexRT[Btn8D] == 1)
		{
			clearPreviousError();
			liftPreset= 3;
		}
		if (vexRT[Btn7U] == 1)
		{
			clearPreviousError();
			liftPreset= 2;
		}
		if (vexRT[Btn7D] == 1)
		{
			clearPreviousError();
			liftPreset= 1;
		}
		if (vexRT[Btn7L] == 1)
		{
			clearPreviousError();
			mountedSkyrise = true;
		}
		if (vexRT[Btn7R] == 1)
		{
			clearPreviousError();
			mountedSkyrise = false;
		}
	}
}

void liftPresetMonitor()
{
	/* Unneccessary code
	if (lift_auton_val[0] == 0)
	{

	}*/

	if (liftAutonVal[0] != 0)
	{
		bool LLgoalReached = false;
		bool RLgoalReached = false;
		string direction;
		if (SensorValue[RL_pot] < liftAutonVal[0])
		{
			direction = "up";
		}
		if (SensorValue[RL_pot] > liftAutonVal[0])
		{
			direction = "down";
		}
		if ((abs(SensorValue[RL_pot]) - 30 < abs(liftAutonVal[0])) && ((abs(SensorValue[RL_pot]) + 30 > abs(liftAutonVal[0]))))
		{
			leftLift = 0;
			rightLift = 0;
			liftAutonVal[0] = 0;
			liftPreset = 0;
		}
		else if ((liftAutonVal[0] != 0))
		{
			if (direction == "down")
			{
				if ((!LLgoalReached) || (!RLgoalReached))
				{
					if (SensorValue[LL_pot] > liftAutonVal[0])
					{
						leftLift = -(liftAutonVal[1]);
						if (liftError < 0)
						{
							leftLift = leftLift + 80;
							if (mountedSkyrise)
							{
								leftLift = leftLift + 120;
							}
						}
					}
					else
					{
						LLgoalReached = true;
						leftLift = 0;
					}
					if (SensorValue[RL_pot] > liftAutonVal[0])
					{
						rightLift = -(liftAutonVal[1]);
						if (liftError > 0)
						{
							rightLift = rightLift + 80;
							if (mountedSkyrise)
							{
								rightLift = rightLift + 120;
							}
						}
					}
					else
					{
						RLgoalReached = true;
						rightLift = 0;
					}
				}
			}
			else if (direction == "up")
			{
				if ((!LLgoalReached) || (!RLgoalReached))
				{
					if (SensorValue[LL_pot] < liftAutonVal[0])
					{
						leftLift = (liftAutonVal[1]);
					if (liftError > 0)
						{
							leftLift = leftLift - 60;
						}
					}
					else
					{
						LLgoalReached = true;
						leftLift = 0;
					}
					if (SensorValue[RL_pot] < liftAutonVal[0])
					{
						rightLift = liftAutonVal[1];
						if (liftError < 0)
						{
							rightLift = rightLift - 60;
						}
					}
					else
					{
						RLgoalReached = true;
						rightLift = 0;
					}
				}
			}
		}
	}
}
//Preset Action
void presetAssign()
{
	switch (liftPreset)
	{
	case 2:
		if (SensorValue[LL_pot] < liftVal[1])
		{
			liftAutonVal[0] = liftVal[1];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	case 1:
		if (SensorValue[LL_pot] > liftVal[0])
		{
			liftAutonVal[0] = liftVal[0];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	case 3:
		if (((SensorValue[LL_pot] < liftVal[2]) || ((SensorValue[LL_pot] > liftVal[2]))))
		{
			liftAutonVal[0] = liftVal[2];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	case 4:
		if (((SensorValue[RL_pot] < liftVal[3]) || ((SensorValue[RL_pot] > liftVal[3]))))
		{
			liftAutonVal[0] = liftVal[3];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	case 5:
		if (((SensorValue[RL_pot] < liftVal[4]) || ((SensorValue[RL_pot] > liftVal[4]))))
		{
			liftAutonVal[0] = liftVal[4];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	case 6:
		if (((SensorValue[RL_pot] < liftVal[4]) || ((SensorValue[RL_pot] > liftVal[4]))))
		{
			liftAutonVal[0] = liftVal[1];
			liftPresetMonitor();
		}
		else
		{
			liftPreset = 0;
		}
		break;
	default:
		break;
	}
}

void pre_auton()
{
	bStopTasksBetweenModes = true;
	clearEncoders();
	clearPreviousError();
}

//User Created Tasks


/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Autonomous Task
//
// This task is used to control your robot during the autonomous phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////
task autonomousCompanion()
{
	//Manages Lifts During Auton
	while(true)
	{
		liftAutonMonitor();
		EndTimeSlice();
	}
}
task autonomous()
{
	StartTask(autonomousCompanion);
	intakeReverse();
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////
/*
I assume this is commented out for a reason, so I'll leave it

task usercontrol_companion()
{
void lift_preset_monitor()
{
	if (lift_auton_val[0] == 0)
	{
		EndTimeSlice();
	}

	else
	{
		bool LL_goal_reached = false;
		bool RL_goal_reached = false;
		target_scale = lift_auton_val[0]/limit_LL_pot;
		int RL_target = (int) round(target_scale*limit_RL_pot);
		string direction;

		if SensorValue[LL_pot] < lift_auton_val[0]
		{
			direction = "up";
		}
		if SensorValue[LL_pot] > lift_auton_val[0]
		{
			direction = "down";
		}

		if ((abs(SensorValue[LL_pot])-30 < abs(lift_auton_val[0])) && ((abs(SensorValue[LL_pot])+30 > abs(lift_auton_val[0]))))
		{
			leftLift = 0;
			rightLift = 0;
			lift_auton_val[0] = 0;
			lift_preset = 0;
		}

		else if ((lift_auton_val[0] != 0))
		{
			while (((LL_goal_reached == false) || (RL_goal_reached = false)) && (direction = "down") && (interrupt = false))
			{
				if (SensorValue[LL_pot] > lift_auton_val[0])
				{
					leftLift = speedstep_lift(lift_auton_val[0],-(lift_auton_val[1]),SensorValue[LL_pot],0);
				}
				else
				{
					LL_goal_reached = true;
					leftLift = 0;
				}
				if (SensorValue[RL_pot] > RL_target)
				{
					rightLift = speedstep_lift(RL_target,-(lift_auton_val[1]),SensorValue[RL_pot],1);
				}
				else
				{
					RL_goal_reached = true;
					rightLift = 0;
				}
			}

			if (((LL_goal_reached == false) || (RL_goal_reached = false)) && (direction = "up") && (interrupt = false))
			{
				if (SensorValue[LL_pot] < lift_auton_val[0])
				{
					leftLift = speedstep_lift(lift_auton_val[0],(lift_auton_val[1]),SensorValue[LL_pot],0);
				}
				else
				{
					LL_goal_reached = true;
					leftLift = 0;
				}
				if (SensorValue[RL_pot] < RL_target)
				{
					rightLift = speedstep_lift(RL_target,lift_auton_val[1],SensorValue[RL_pot],1);
				}
				else
				{
					RL_goal_reached = true;
					rightLift = 0;
				}
			}
		}

	}
}
} */

task usercontrol()
{
	//Resetting auton variables for usercontrol
	liftAutonVal[0] = 0;
	liftAutonVal[1] = 127;
	clearPreviousError();

	while(true)
	{
		//Buttons
				presetButtons();
		//Drive
			//Left Stick Control (Strafe and movement)
				leftDF = vexRT[Ch3] + vexRT[Ch4];
				leftDB = vexRT[Ch3] - vexRT[Ch4];
				rightDF  =vexRT[Ch3] - vexRT[Ch4];
				rightDB = vexRT[Ch3] + vexRT[Ch4];
			//Right Stick Control (Turning)
				leftDF = leftDF + vexRT[Ch1];
				leftDB = leftDB + vexRT[Ch1];
				rightDF = rightDF - vexRT[Ch1];
				rightDB = rightDB - vexRT[Ch1];
			//apply exp control
				leftDF = expCtrl(leftDF, expScalingVal);
				leftDB = expCtrl(leftDB, expScalingVal);
				rightDF = expCtrl(rightDF, expScalingVal);
				rightDB = expCtrl(rightDB, expScalingVal);
			//Give motors values
				motor[LDF] = leftDF;
				motor[LDB] = leftDB;
				motor[RDF] = rightDF;
				motor[RDB] = rightDB;
		//Lift
			//Get Controller Values
				leftLift = (vexRT[Btn5U] - vexRT[Btn5D]) * 127;
				rightLift = (vexRT[Btn5U] - vexRT[Btn5D]) * 127;
				if ((abs(leftLift) > 0) || (abs(rightLift) > 0))
				{
					liftPreset = 0;
					interrupt = true;
					if (vexRT[Btn5U] == 1)
					{
						if (liftError < 0)
						{
							rightLift = rightLift - 60;
						}
						else if (liftError > 0)
						{
							leftLift = leftLift - 60;
						}
					}
					if (vexRT[Btn5D] == 1)
					{
						if (liftError < 0)
						{
							leftLift = leftLift + 80;
							if (mountedSkyrise)
							{
								leftLift = leftLift + 120;
							}
						}
						else if (liftError > 0)
						{
							rightLift = rightLift + 80;
							if (mountedSkyrise)
							{
								rightLift = rightLift + 120;
							}
						}
					}
				}
				else
				{
					//Lift Preset Actions
					if (liftPreset != 0)
					{
						interrupt = false;
						presetAssign();
					}
					else
					//Trim if idle
					{
						if (SensorValue[RL_pot] < trimSwitch)
						{
							leftLift = leftLift - 30;
							rightLift = rightLift - 30;
						}
						else if (SensorValue[RL_pot] > trimSwitch)
						{
							leftLift = leftLift + 20;
							rightLift = rightLift + 20;
						}
					}
				}
			//Give Lift Values
				motor[LLD] = leftLift;
				motor[LLU] = leftLift;
				motor[RLD] = rightLift;
				motor[RLU] = rightLift;
		//Intake
				motor[LIN] = (vexRT[Btn6U] - vexRT[Btn6D]) * 127;
				motor[RIN] = (vexRT[Btn6U] - vexRT[Btn6D]) * 127;

	}
}
