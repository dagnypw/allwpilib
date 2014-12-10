/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2014. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/

#include "CANTalon.h"
#include "WPIErrors.h"
#include "ctre/CanTalonSRX.h"
#include <unistd.h> // usleep
/**
 * Constructor for the CANTalon device.
 * @param deviceNumber The CAN ID of the Talon SRX
 */
CANTalon::CANTalon(int deviceNumber)
	: m_deviceNumber(deviceNumber)
	, m_impl(new CanTalonSRX(deviceNumber))
	, m_safetyHelper(new MotorSafetyHelper(this))
  , m_profile(0)
  , m_controlEnabled(true)
  , m_controlMode(kPercentVbus)
{
  SetControlMode(m_controlMode);
  m_impl->SetProfileSlotSelect(m_profile);
}

CANTalon::~CANTalon() {
	delete m_impl;
	delete m_safetyHelper;
}

/**
* Write out the PID value as seen in the PIDOutput base object.
*
* @deprecated Call Set instead.
*
* @param output Write out the PercentVbus value as was computed by the PIDController
*/
void CANTalon::PIDWrite(float output)
{
	if (GetControlMode() == kPercentVbus)
	{
		Set(output);
	}
	else
	{
		wpi_setWPIErrorWithContext(IncompatibleMode, "PID only supported in PercentVbus mode");
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
float CANTalon::Get()
{
  int value;
  switch(m_controlMode) {
    case kVoltage:
      return GetOutputVoltage();
    case kCurrent:
      return GetOutputCurrent();
    case kSpeed:
      m_impl->GetSensorVelocity(value);
      return value;
    case kPosition:
      m_impl->GetSensorPosition(value);
      return value;
    case kPercentVbus:
    default:
      m_impl->GetAppliedThrottle(value);
      return (float)value / 1023.0;
  }
}

/**
 * Sets the appropriate output on the talon, depending on the mode.
 *
 * In PercentVbus, the output is between -1.0 and 1.0, with 0.0 as stopped.
 * In Voltage mode, outputValue is in volts.
 * In Current mode, outputValue is in amperes.
 * In Speed mode, outputValue is in position change / 10ms.
 * In Position mode, outputValue is in encoder ticks or an analog value,
 *   depending on the sensor.
 *
 * @param outputValue The setpoint value, as described above.
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
void CANTalon::Set(float value, uint8_t syncGroup)
{
  if(m_controlEnabled) {
    CTR_Code status;
    switch(m_controlMode) {
      case CANSpeedController::kPercentVbus:
        {
          m_impl->Set(value);
          status = CTR_OKAY;
        }
        break;
      case CANSpeedController::kFollower:
        {
          status = m_impl->SetDemand((int)value);
        }
        break;
      case CANSpeedController::kVoltage:
        {
          // Voltage is an 8.8 fixed point number.
          int volts = int(value * 256);
          status = m_impl->SetDemand(volts);
        }
        break;
      case CANSpeedController::kSpeed:
        status = m_impl->SetDemand(value);
        break;
      case CANSpeedController::kPosition:
        status = m_impl->SetDemand(value);
        break;
      default:
        break;
    }
    if (status != CTR_OKAY) {
      wpi_setErrorWithContext(status, getHALErrorMessage(status));
    }

    status = m_impl->SetModeSelect(m_sendMode);

    if (status != CTR_OKAY) {
      wpi_setErrorWithContext(status, getHALErrorMessage(status));
    }

  }
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::Disable()
{
  // Until Modes other than throttle work, just disable by setting throttle to 0.0.
  m_impl->SetModeSelect((int)CANTalon::kDisabled);
  m_controlEnabled = false;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::EnableControl() {
  SetControlMode(m_controlMode);
  m_controlEnabled = true;
}

/**
 * @param p Proportional constant to use in PID loop.
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
void CANTalon::SetP(double p)
{
  CTR_Code status = m_impl->SetPgain(m_profile, p);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
void CANTalon::SetI(double i)
{
  CTR_Code status = m_impl->SetIgain(m_profile, i);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
void CANTalon::SetD(double d)
{
  CTR_Code status = m_impl->SetDgain(m_profile, d);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}
/**
 *
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
void CANTalon::SetF(double f)
{
  CTR_Code status = m_impl->SetFgain(m_profile, f);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}
/**
 * SRX has two available slots for PID.
 * @param slotIdx one or zero depending on which slot caller wants.
 */
void CANTalon::SelectProfileSlot(int slotIdx)
{
	m_profile = (slotIdx == 0) ? 0 : 1; /* only get two slots for now */
	CTR_Code status = m_impl->SetProfileSlotSelect(m_profile);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}
/**
 * TODO documentation (see CANJaguar.cpp)
 * This function does not modify F-gain.  Considerable passing a zero for f using
 * the four-parameter function.
 */
void CANTalon::SetPID(double p, double i, double d)
{
	SetP(p);
	SetI(i);
	SetD(d);
}
void CANTalon::SetPID(double p, double i, double d, double f)
{
	SetP(p);
	SetI(i);
	SetD(d);
	SetF(f);
}
/**
 * Select the feedback device to use in closed-loop
 */
void CANTalon::SetFeedbackDevice(FeedbackDevice device)
{
	CTR_Code status = m_impl->SetFeedbackDeviceSelect((int)device);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
double CANTalon::GetP()
{
  CanTalonSRX::param_t param = m_profile ? CanTalonSRX::eProfileParamSlot1_P : CanTalonSRX::eProfileParamSlot0_P;
  // Update the info in m_impl.
  CTR_Code status = m_impl->RequestParam(param);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
  }
  usleep(1000); /* small yield for getting response */
  double p;
  status = m_impl->GetPgain(m_profile, p);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return p;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
double CANTalon::GetI()
{
  CanTalonSRX::param_t param = m_profile ? CanTalonSRX::eProfileParamSlot1_I : CanTalonSRX::eProfileParamSlot0_I;
  // Update the info in m_impl.
  CTR_Code status = m_impl->RequestParam(param);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
  }
	usleep(1000); /* small yield for getting response */

  double i;
  status = m_impl->GetIgain(m_profile, i);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return i;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
double CANTalon::GetD()
{
  CanTalonSRX::param_t param = m_profile ? CanTalonSRX::eProfileParamSlot1_D : CanTalonSRX::eProfileParamSlot0_D;
  // Update the info in m_impl.
  CTR_Code status = m_impl->RequestParam(param);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
  }
	usleep(1000); /* small yield for getting response */

  double d;
  status = m_impl->GetDgain(m_profile, d);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return d;
}
/**
 *
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
double CANTalon::GetF()
{
  CanTalonSRX::param_t param = m_profile ? CanTalonSRX::eProfileParamSlot1_F : CanTalonSRX::eProfileParamSlot0_F;
  // Update the info in m_impl.
  CTR_Code status = m_impl->RequestParam(param);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
  }

	usleep(1000);  /* small yield for getting response */
  double f;
  status = m_impl->GetFgain(m_profile, f);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return f;
}
/**
 * @see SelectProfileSlot to choose between the two sets of gains.
 */
double CANTalon::GetIzone()
{
  CanTalonSRX::param_t param = m_profile ? CanTalonSRX::eProfileParamSlot1_IZone: CanTalonSRX::eProfileParamSlot0_IZone;
 // Update the info in m_impl.
 CTR_Code status = m_impl->RequestParam(param);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
 }
	usleep(1000);

 int iz;
 status = m_impl->GetIzone(m_profile, iz);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return (double)iz;
}

/**
 * Returns the voltage coming in from the battery.
 *
 * @return The input voltage in vols.
 */
float CANTalon::GetBusVoltage()
{
  double voltage;
  CTR_Code status = m_impl->GetBatteryV(voltage);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return voltage;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
float CANTalon::GetOutputVoltage()
{
  int throttle11;
  CTR_Code status = m_impl->GetAppliedThrottle(throttle11);
  float voltage = GetBusVoltage() * (float(throttle11) / 1023.0);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return voltage;
}


/**
 * TODO documentation (see CANJaguar.cpp)
 */
float CANTalon::GetOutputCurrent()
{
  double current;

  CTR_Code status = m_impl->GetCurrent(current);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}

	return current;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
float CANTalon::GetTemperature()
{
  double temp;

  CTR_Code status = m_impl->GetTemp(temp);
	if(temp != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return temp;
}
/**
 * Set the position value of the selected sensor.  This is useful for zero-ing quadrature encoders.
 * Continuous sensors (like analog encoderes) can also partially be set (the portion of the postion based on overflows).
 */
void CANTalon::SetPosition(double pos)
{
	m_impl->SetSensorPosition(pos);
}
/**
 * TODO documentation (see CANJaguar.cpp)
 *
 * @return The position of the sensor currently providing feedback.
 * 			When using analog sensors, 0 units corresponds to 0V, 1023 units corresponds to 3.3V
 * 			When using an analog encoder (wrapping around 1023 => 0 is possible) the units are still 3.3V per 1023 units.
 * 			When using quadrature, each unit is a quadrature edge (4X) mode.
 */
double CANTalon::GetPosition()
{
  int postition;

  CTR_Code status = m_impl->GetSensorPosition(postition);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return (double)postition;
}
/**
 * If sensor and motor are out of phase, sensor can be inverted 
 * (position and velocity multiplied by -1).
 * @see GetPosition and @see GetSpeed.
 */
void CANTalon::SetSensorDirection(bool reverseSensor)
{
  CTR_Code status = m_impl->SetRevFeedbackSensor(reverseSensor ? 1 : 0);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * Returns the current error in the controller.
 *
 * @return the difference between the setpoint and the sensor value.
 */
int CANTalon::GetClosedLoopError() {
  int error;
  CTR_Code status = m_impl->GetCloseLoopErr(error);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return error;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 *
 * @returns The speed of the sensor currently providing feedback.
 *
 * The speed units will be in the sensor's native ticks per 100ms.
 *
 * For analog sensors, 3.3V corresponds to 1023 units.
 * 		So a speed of 200 equates to ~0.645 dV per 100ms or 6.451 dV per second.
 * 		If this is an analog encoder, that likely means 1.9548 rotations per sec.
 * For quadrature encoders, each unit corresponds a quadrature edge (4X).
 * 		So a 250 count encoder will produce 1000 edge events per rotation.
 * 		An example speed of 200 would then equate to 20% of a rotation per 100ms,
 * 		or 10 rotations per second.
 */
double CANTalon::GetSpeed()
{
  int speed;
  // TODO convert from int to appropriate units (or at least document it).

  CTR_Code status = m_impl->GetSensorVelocity(speed);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return (double)speed;
}

/**
 * Get the position of whatever is in the analog pin of the Talon, regardless of
 * whether it is actually being used for feedback.
 *
 * @returns The value (0 - 1023) on the analog pin of the Talon.
 */
int CANTalon::GetAnalogIn()
{
  int position;
  CTR_Code status = m_impl->GetAnalogInWithOv(position);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return position;
}

/**
 * Get the position of whatever is in the analog pin of the Talon, regardless of
 * whether it is actually being used for feedback.
 *
 * @returns The value (0 - 1023) on the analog pin of the Talon.
 */
int CANTalon::GetAnalogInVel()
{
  int vel;
  CTR_Code status = m_impl->GetAnalogInVel(vel);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return vel;
}

/**
 * Get the position of whatever is in the analog pin of the Talon, regardless of
 * whether it is actually being used for feedback.
 *
 * @returns The value (0 - 1023) on the analog pin of the Talon.
 */
int CANTalon::GetEncPosition()
{
  int position;
  CTR_Code status = m_impl->GetEncPosition(position);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return position;
}

/**
 * Get the position of whatever is in the analog pin of the Talon, regardless of
 * whether it is actually being used for feedback.
 *
 * @returns The value (0 - 1023) on the analog pin of the Talon.
 */
int CANTalon::GetEncVel()
{
  int vel;
  CTR_Code status = m_impl->GetEncVel(vel);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
  return vel;
}
/**
 * @return IO level of QUADA pin.
 */
int CANTalon::GetPinStateQuadA()
{
	int retval;
	CTR_Code status = m_impl->GetQuadApin(retval);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return retval;
}
/**
 * @return IO level of QUADB pin.
 */
int CANTalon::GetPinStateQuadB()
{
	int retval;
	CTR_Code status = m_impl->GetQuadBpin(retval);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return retval;
}
/**
 * @return IO level of QUAD Index pin.
 */
int CANTalon::GetPinStateQuadIdx()
{
	int retval;
	CTR_Code status = m_impl->GetQuadIdxpin(retval);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return retval;
}
/**
 * @return '1' iff forward limit switch is closed, 0 iff switch is open.
 * This function works regardless if limit switch feature is enabled.
 */
int CANTalon::IsFwdLimitSwitchClosed()
{
	int retval;
	CTR_Code status = m_impl->GetLimitSwitchClosedFor(retval); /* rename this func, '1' => open, '0' => closed */
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return retval ? 0 : 1;
}
/**
 * @return '1' iff reverse limit switch is closed, 0 iff switch is open.
 * This function works regardless if limit switch feature is enabled.
 */
int CANTalon::IsRevLimitSwitchClosed()
{
	int retval;
	CTR_Code status = m_impl->GetLimitSwitchClosedRev(retval); /* rename this func, '1' => open, '0' => closed */
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return retval ? 0 : 1;
}
/*
 * Simple accessor for tracked rise eventso index pin.
 * @return number of rising edges on idx pin.
 */
int CANTalon::GetNumberOfQuadIdxRises()
{
	int rises;
	CTR_Code status = m_impl->GetEncIndexRiseEvents(rises); /* rename this func, '1' => open, '0' => closed */
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return rises;
}
/*
 * @param rises integral value to set into index-rises register.  Great way to zero the index count.
 */
void CANTalon::SetNumberOfQuadIdxRises(int rises)
{
	CTR_Code status = m_impl->SetParam(CanTalonSRX::eEncIndexRiseEvents, rises); /* rename this func, '1' => open, '0' => closed */
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}
/**
 * TODO documentation (see CANJaguar.cpp)
 */
bool CANTalon::GetForwardLimitOK()
{
	int limSwit=0;
	int softLim=0;
	CTR_Code status;
	status = m_impl->GetFault_ForSoftLim(softLim);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	status = m_impl->GetFault_ForLim(limSwit);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	/* If either fault is asserted, signal caller we are disabled (with false?) */
	return (softLim | limSwit) ? false : true;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
bool CANTalon::GetReverseLimitOK()
{
	int limSwit=0;
	int softLim=0;
	CTR_Code status;
	status = m_impl->GetFault_RevSoftLim(softLim);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	status = m_impl->GetFault_RevLim(limSwit);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	/* If either fault is asserted, signal caller we are disabled (with false?) */
	return (softLim | limSwit) ? false : true;
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
uint16_t CANTalon::GetFaults()
{
	uint16_t retval = 0;
	int val;
	CTR_Code status;

	/* temperature */
	val = 0;
	status = m_impl->GetFault_OverTemp(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kTemperatureFault : 0;

	/* voltage */
	val = 0;
	status = m_impl->GetFault_UnderVoltage(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kBusVoltageFault : 0;

	/* fwd-limit-switch */
	val = 0;
	status = m_impl->GetFault_ForLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kFwdLimitSwitch : 0;

	/* rev-limit-switch */
	val = 0;
	status = m_impl->GetFault_RevLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kRevLimitSwitch : 0;

	/* fwd-soft-limit */
	val = 0;
	status = m_impl->GetFault_ForSoftLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kFwdSoftLimit : 0;

	/* rev-soft-limit */
	val = 0;
	status = m_impl->GetFault_RevSoftLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kRevSoftLimit : 0;

	return retval;
}
uint16_t CANTalon::GetStickyFaults()
{
	uint16_t retval = 0;
	int val;
	CTR_Code status;

	/* temperature */
	val = 0;
	status = m_impl->GetStckyFault_OverTemp(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kTemperatureFault : 0;

	/* voltage */
	val = 0;
	status = m_impl->GetStckyFault_UnderVoltage(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kBusVoltageFault : 0;

	/* fwd-limit-switch */
	val = 0;
	status = m_impl->GetStckyFault_ForLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kFwdLimitSwitch : 0;

	/* rev-limit-switch */
	val = 0;
	status = m_impl->GetStckyFault_RevLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kRevLimitSwitch : 0;

	/* fwd-soft-limit */
	val = 0;
	status = m_impl->GetStckyFault_ForSoftLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kFwdSoftLimit : 0;

	/* rev-soft-limit */
	val = 0;
	status = m_impl->GetStckyFault_RevSoftLim(val);
	if(status != CTR_OKAY)
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	retval |= (val) ? CANSpeedController::kRevSoftLimit : 0;

	return retval;
}
void CANTalon::ClearStickyFaults()
{
	CTR_Code status = m_impl->ClearStickyFaults();
	wpi_setErrorWithContext(status, getHALErrorMessage(status));
}

/**
 * Set the maximum voltage change rate.  This ramp rate is in affect regardless of which control mode
 * the TALON is in.
 *
 * When in PercentVbus or Voltage output mode, the rate at which the voltage changes can
 * be limited to reduce current spikes.  Set this to 0.0 to disable rate limiting.
 *
 * @param rampRate The maximum rate of voltage change in Percent Voltage mode in V/s.
 */
void CANTalon::SetVoltageRampRate(double rampRate)
{
	/* Caller is expressing ramp as Voltage per sec, assuming 12V is full.
		Talon's throttle ramp is in dThrot/d10ms.  1023 is full fwd, -1023 is full rev. */
	double rampRatedThrotPer10ms = (rampRate*1023.0/12.0) / 100;
	CTR_Code status = m_impl->SetRampThrottle((int)rampRatedThrotPer10ms);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}
/**
 * Sets a voltage change rate that applies only when a close loop contorl mode is enabled.
 * This allows close loop specific ramp behavior.
 *
 * @param rampRate The maximum rate of voltage change in Percent Voltage mode in V/s.
 */
void CANTalon::SetCloseLoopRampRate(double rampRate)
{
	  CTR_Code status = m_impl->SetCloseLoopRampRate(m_profile,rampRate * 1023.0 / 12.0 / 1000.0);
		if(status != CTR_OKAY) {
			wpi_setErrorWithContext(status, getHALErrorMessage(status));
		}
}

/**
 * @return The version of the firmware running on the Talon
 */
uint32_t CANTalon::GetFirmwareVersion()
{
	int firmwareVersion;
	CTR_Code status = m_impl->RequestParam(CanTalonSRX::eFirmVers);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	usleep(1000);
	status = m_impl->GetParamResponseInt32(CanTalonSRX::eFirmVers,firmwareVersion);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}

	/* only sent once on boot */
	//CTR_Code status = m_impl->GetFirmVers(firmwareVersion);
	//if(status != CTR_OKAY) {
	//	wpi_setErrorWithContext(status, getHALErrorMessage(status));
	//}

	return firmwareVersion;
}
/**
 * @return The accumulator for I gain.
 */
int CANTalon::GetIaccum()
{
	CTR_Code status = m_impl->RequestParam(CanTalonSRX::ePidIaccum);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	usleep(1000); /* small yield for getting response */
	int iaccum;
	status = m_impl->GetParamResponseInt32(CanTalonSRX::ePidIaccum,iaccum);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
	return iaccum;
}
/**
 * Clear the accumulator for I gain.
 */
void CANTalon::ClearIaccum()
{
	CTR_Code status = m_impl->SetParam(CanTalonSRX::ePidIaccum, 0);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigNeutralMode(NeutralMode mode)
{
	CTR_Code status;
	switch(mode){
		default:
		case kNeutralMode_Jumper: /* use default setting in flash based on webdash/BrakeCal button selection */
			status = m_impl->SetOverrideBrakeType(CanTalonSRX::kBrakeOverride_UseDefaultsFromFlash);
			break;
		case kNeutralMode_Brake:
			status = m_impl->SetOverrideBrakeType(CanTalonSRX::kBrakeOverride_OverrideBrake);
			break;
		case kNeutralMode_Coast:
			status = m_impl->SetOverrideBrakeType(CanTalonSRX::kBrakeOverride_OverrideCoast);
			break;
	}
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigEncoderCodesPerRev(uint16_t codesPerRev)
{
	/* TALON SRX does not scale units, they are raw from the sensor.  Unit scaling can be done in API or by caller */
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigPotentiometerTurns(uint16_t turns)
{
	/* TALON SRX does not scale units, they are raw from the sensor.  Unit scaling can be done in API or by caller */
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigSoftPositionLimits(double forwardLimitPosition, double reverseLimitPosition)
{
	ConfigLimitMode(kLimitMode_SoftPositionLimits);
	ConfigForwardLimit(forwardLimitPosition);
	ConfigReverseLimit(reverseLimitPosition);
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::DisableSoftPositionLimits()
{
	ConfigLimitMode(kLimitMode_SwitchInputsOnly);
}

/**
 * TODO documentation (see CANJaguar.cpp)
 * Configures the soft limit enable (wear leveled persistent memory).
 * Also sets the limit switch overrides.
 */
void CANTalon::ConfigLimitMode(LimitMode mode)
{
	CTR_Code status;
	switch(mode){
		case kLimitMode_SwitchInputsOnly: 	/** Only use switches for limits */
			/* turn OFF both limits. SRX has individual enables and polarity for each limit switch.*/
			status = m_impl->SetForwardSoftEnable(false);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			status = m_impl->SetReverseSoftEnable(false);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			/* override enable the limit switches, this circumvents the webdash */
			status = m_impl->SetOverrideLimitSwitchEn(CanTalonSRX::kLimitSwitchOverride_EnableFwd_EnableRev);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			break;
		case kLimitMode_SoftPositionLimits: /** Use both switches and soft limits */
			/* turn on both limits. SRX has individual enables and polarity for each limit switch.*/
			status = m_impl->SetForwardSoftEnable(true);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			status = m_impl->SetReverseSoftEnable(true);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			/* override enable the limit switches, this circumvents the webdash */
			status = m_impl->SetOverrideLimitSwitchEn(CanTalonSRX::kLimitSwitchOverride_EnableFwd_EnableRev);
			if(status != CTR_OKAY) {
				wpi_setErrorWithContext(status, getHALErrorMessage(status));
			}
			break;
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigForwardLimit(double forwardLimitPosition)
{
	CTR_Code status;
	status = m_impl->SetForwardSoftLimit(forwardLimitPosition);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigReverseLimit(double reverseLimitPosition)
{
	CTR_Code status;
	status = m_impl->SetReverseSoftLimit(reverseLimitPosition);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigMaxOutputVoltage(double voltage)
{
	/* SRX does not support max output  */
	wpi_setWPIErrorWithContext(IncompatibleMode, "MaxOutputVoltage not supported.");
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::ConfigFaultTime(float faultTime)
{
	/* SRX does not have fault time.  SRX motor drive is only disabled for soft limits and limit-switch faults. */
	wpi_setWPIErrorWithContext(IncompatibleMode, "Fault Time not supported.");
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
void CANTalon::SetControlMode(CANSpeedController::ControlMode mode)
{
  m_controlMode = mode;
  switch (mode) {
    case kPercentVbus:
      m_sendMode = kThrottle;
      break;
    case kCurrent:
      m_sendMode = kCurrentMode;
      break;
    case kSpeed:
      m_sendMode = kSpeedMode;
      break;
    case kPosition:
      m_sendMode = kPositionMode;
      break;
    case kVoltage:
      m_sendMode = kVoltageMode;
      break;
    case kFollower:
      m_sendMode = kFollowerMode;
      break;
  }
  // Keep the talon disabled until Set() is called.
  CTR_Code status = m_impl->SetModeSelect((int)kDisabled);
	if(status != CTR_OKAY) {
		wpi_setErrorWithContext(status, getHALErrorMessage(status));
	}
}

/**
 * TODO documentation (see CANJaguar.cpp)
 */
CANSpeedController::ControlMode CANTalon::GetControlMode()
{
  return m_controlMode;
}

void CANTalon::SetExpiration(float timeout)
{
	m_safetyHelper->SetExpiration(timeout);
}

float CANTalon::GetExpiration()
{
	return m_safetyHelper->GetExpiration();
}

bool CANTalon::IsAlive()
{
	return m_safetyHelper->IsAlive();
}

bool CANTalon::IsSafetyEnabled()
{
	return m_safetyHelper->IsSafetyEnabled();
}

void CANTalon::SetSafetyEnabled(bool enabled)
{
	m_safetyHelper->SetSafetyEnabled(enabled);
}

void CANTalon::GetDescription(char *desc)
{
	sprintf(desc, "CANTalon ID %d", m_deviceNumber);
}

/**
 * Common interface for stopping the motor
 * Part of the MotorSafety interface
 *
 * @deprecated Call Disable instead.
*/
void CANTalon::StopMotor()
{
  Disable();
}