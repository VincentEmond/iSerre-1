/*
 * SinkMeasureManager.cpp
 *
 *  Created on: 16 déc. 2016
 *      Author: Julien
 */

#include "SinkMeasureManager.h"

SinkMeasureManager::SinkMeasureManager(int measureNeededForAverage) :
	m_averageDone(false), m_currentSum(0), m_average(0), m_currentMeasureNumber(0),
	m_measureNumber(measureNeededForAverage)
{

}

float SinkMeasureManager::getAverage()
{
	m_averageDone = false;
	return m_average;
}

void SinkMeasureManager::addMeasure(float measure)
{
	m_currentSum += measure;
	m_currentMeasureNumber++;
	if(m_currentMeasureNumber >= m_measureNumber)
	{
		m_average = m_currentSum/m_measureNumber;
		m_currentSum = 0.f;
		m_currentMeasureNumber = 0;
		m_averageDone = true;
	}
}

bool SinkMeasureManager::isAverageDone() const
{
	return m_averageDone;
}

void SinkMeasureManager::setMeasureCountNeededForAverage(int measureCount)
{
	m_measureNumber = measureCount;
}

