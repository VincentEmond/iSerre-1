/*
 * SinkMeasureManager.h
 *
 *  Created on: 16 déc. 2016
 *      Author: Julien
 */

#ifndef SOURCES_SINKMEASUREMANAGER_H_
#define SOURCES_SINKMEASUREMANAGER_H_

class SinkMeasureManager
{
public:
	SinkMeasureManager(int measureNeededForAverage);

	float getAverage();

	void addMeasure(float measure);

	bool isAverageDone() const;

	void setMeasureCountNeededForAverage(int measureCount);

private:
	bool m_averageDone;
	float m_currentSum;
	float m_average;
	int m_currentMeasureNumber;
	int m_measureNumber;
};

#endif /* SOURCES_SINKMEASUREMANAGER_H_ */
