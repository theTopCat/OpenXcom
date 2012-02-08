/*
 * Copyright 2010 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _USE_MATH_DEFINES
#include "MovingTarget.h"
#include <cmath>

namespace OpenXcom
{

/**
 * Initializes a moving target with blank coordinates.
 */
MovingTarget::MovingTarget() : Target(), _dest(0), _speedLon(0.0), _speedLat(0.0), _radianSpeed(0.0), _distCurrent(0.0), _distMax(0.0), _speed(0)
{
}

MovingTarget::~MovingTarget()
{
}

/**
 * Loads the moving target from a YAML file.
 * @param node YAML node.
 */
void MovingTarget::load(const YAML::Node &node)
{
	Target::load(node);
	node["speedLon"] >> _speedLon;
	node["speedLat"] >> _speedLat;
	node["speed"] >> _speed;
}

/**
 * Saves the moving target to a YAML file.
 * @param out YAML emitter.
 */
void MovingTarget::save(YAML::Emitter &out) const
{
	Target::save(out);
	if (_dest != 0)
	{
		out << YAML::Key << "dest" << YAML::Value;
		_dest->saveId(out);
	}
	out << YAML::Key << "speedLon" << YAML::Value << _speedLon;
	out << YAML::Key << "speedLat" << YAML::Value << _speedLat;
	out << YAML::Key << "speed" << YAML::Value << _speed;
}

/**
 * Returns the destination the moving target is heading to.
 * @return Pointer to destination.
 */
Target *const MovingTarget::getDestination() const
{
	return _dest;
}

/**
 * Changes the destination the moving target is heading to.
 * @param dest Pointer to destination.
 */
void MovingTarget::setDestination(Target *dest)
{
	// Remove moving target from old destination's followers
	if (_dest != 0)
	{
		for (std::vector<Target*>::iterator i = _dest->getFollowers()->begin(); i != _dest->getFollowers()->end(); ++i)
		{
			if ((*i) == this)
			{
				_dest->getFollowers()->erase(i);
				break;
			}
		}
	}
	_dest = dest;
	// Add moving target to new destination's followers
	if (_dest != 0)
	{
		_dest->getFollowers()->push_back(this);
		_distMax = getDistance(_dest);
		_distCurrent = 0.0;
	}
	else
	{
		_distMax = _distCurrent = 0.0;
	}
	calculateSpeed();
}

/**
 * Returns the speed of the moving target.
 * @return Speed in knots.
 */
int MovingTarget::getSpeed() const
{
	return _speed;
}

/**
 * Changes the speed of the moving target
 * and converts it from standard knots (nautical miles per hour)
 * into radians per 5 in-game seconds.
 * @param speed Speed in knots.
 */
void MovingTarget::setSpeed(int speed)
{
	_speed = speed;
	// Each nautical mile is 1/60th of a degree.
	// Each hour contains 720 5-seconds.
	_radianSpeed = _speed * (1 / 60.0) * (M_PI / 180) / 720.0;
	calculateSpeed();
}

/**
 * Returns the great circle distance to another
 * target on the globe.
 * @param target Pointer to other target.
 * @returns Distance in radian.
 */
double MovingTarget::getDistance(Target *target) const
{
	return acos(cos(_lat) * cos(target->getLatitude()) * cos(target->getLongitude() - _lon) + sin(_lat) * sin(target->getLatitude()));
}

/**
 * Calculates the speed vector based on the
 * great circle distance to destination and
 * current raw speed.
 */
void MovingTarget::calculateSpeed()
{
	if (_dest != 0)
	{
		double dLon, dLat, length;
		dLon = sin(_dest->getLongitude() - _lon) * cos(_dest->getLatitude());
		dLat = cos(_lat) * sin(_dest->getLatitude()) - sin(_lat) * cos(_dest->getLatitude()) * cos(_dest->getLongitude() - _lon);
		length = sqrt(dLon * dLon + dLat * dLat);
		_speedLon = dLon / length * _radianSpeed / cos(_lat + _speedLat);
		_speedLat = dLat / length * _radianSpeed;
	}
	else
	{
		_speedLon = 0;
		_speedLat = 0;
	}
}

/**
 * Checks if the moving target has finished its route by checking
 * if it has exceeded the destination position based on the speed vector.
 * @return True if it has, False otherwise.
 */
bool MovingTarget::finishedRoute() const
{
	return (_distCurrent >= _distMax);
}

/**
 * Checks if the moving target has reached its destination.
 * @return True if it has, False otherwise.
 */
bool MovingTarget::reachedDestination() const
{
	if (_dest == 0)
	{
		return false;
	}
	return (_lon == _dest->getLongitude() && _lat == _dest->getLatitude());
}

/**
 * Executes a movement cycle for the moving target.
 */
void MovingTarget::move()
{
	setLongitude(_lon + _speedLon);
	setLatitude(_lat + _speedLat);
	_distCurrent += _radianSpeed;
}

}
