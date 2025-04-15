#pragma once
#include <stdlib.h>
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <stdio.h>
#define MSEC_DIV (CLOCKS_PER_SEC/1000);
#endif

#define DEFAULT_DURATION 250
#define SCROLL_MODE 0
#define FLING_MODE 1
#define INFLEXION 0.35f // Tension lines cross at (INFLEXION, 1)
#define START_TENSION 0.5f
#define END_TENSION 1.0f
#define P1 (START_TENSION * INFLEXION)
#define P2 (1.0f - END_TENSION * (1.0f - INFLEXION))
#define NB_SAMPLES 1024
namespace RDPDFLib
{
	namespace view
	{
		class DXScroller
		{
		private:
#define VISCOUS_FLUID_SCALE 8.0f
			static float VISCOUS_FLUID_NORMALIZE;
			static float VISCOUS_FLUID_OFFSET;
			static inline float viscousFluid(float x)
			{
				x *= VISCOUS_FLUID_SCALE;
				if (x < 1.0f) {
					x -= (1.0f - (float)exp(-x));
				}
				else {
					float start = 0.36787944117f;   // 1/e == exp(-1)
					x = 1.0f - (float)exp(1.0f - x);
					x = start + x * (1.0f - start);
				}
				return x;
			}
			static inline int signum(float x)
			{
				if (x > 0) return 1;
				if (x < 0) return -1;
				return 0;
			}
			static inline long long getTickCount()
			{
#ifdef _WIN32
				return ::GetTickCount();
#else
				return clock() / MSEC_DIV;
#endif
			}
			static inline float getInterpolation(float input)
			{
				float interpolated = VISCOUS_FLUID_NORMALIZE * viscousFluid(input);
				if (interpolated > 0) {
					return interpolated + VISCOUS_FLUID_OFFSET;
				}
				return interpolated;
			}
			static float SPLINE_POSITION[NB_SAMPLES + 1];
			static float SPLINE_TIME[NB_SAMPLES + 1];
			static float DECELERATION_RATE;
			static bool ms_init;
			int mMode;
			int mStartX;
			int mStartY;
			int mFinalX;
			int mFinalY;
			int mMinX;
			int mMaxX;
			int mMinY;
			int mMaxY;
			int mCurrX;
			int mCurrY;
			long long mStartTime;
			int mDuration;
			float mDurationReciprocal;
			float mDeltaX;
			float mDeltaY;
			bool mFinished;
			bool mFlywheel;
			float mVelocity;
			float mCurrVelocity;
			int mDistance;
			float mFlingFriction = 0.015f;
			float mDeceleration;
			float mPpi;
			float mPhysicalCoeff;

			static void init()
			{
				if (ms_init) return;
				ms_init = true;
				VISCOUS_FLUID_NORMALIZE = 1.0f / viscousFluid(1.0f);
				VISCOUS_FLUID_OFFSET = 1.0f - VISCOUS_FLUID_NORMALIZE * viscousFluid(1.0f);

				float x_min = 0.0f;
				float y_min = 0.0f;
				for (int i = 0; i < NB_SAMPLES; i++)
				{
					float alpha = (float)i / NB_SAMPLES;

					float x_max = 1.0f;
					float x, tx, coef;
					while (true)
					{
						x = x_min + (x_max - x_min) / 2.0f;
						coef = 3.0f * x * (1.0f - x);
						tx = coef * ((1.0f - x) * P1 + x * P2) + x * x * x;
						if (fabs(tx - alpha) < 1E-5) break;
						if (tx > alpha) x_max = x;
						else x_min = x;
					}
					SPLINE_POSITION[i] = coef * ((1.0f - x) * START_TENSION + x) + x * x * x;

					float y_max = 1.0f;
					float y, dy;
					while (true) {
						y = y_min + (y_max - y_min) / 2.0f;
						coef = 3.0f * y * (1.0f - y);
						dy = coef * ((1.0f - y) * START_TENSION + y) + y * y * y;
						if (fabs(dy - alpha) < 1E-5) break;
						if (dy > alpha) y_max = y;
						else y_min = y;
					}
					SPLINE_TIME[i] = coef * ((1.0f - y) * P1 + y * P2) + y * y * y;
				}
				SPLINE_POSITION[NB_SAMPLES] = SPLINE_TIME[NB_SAMPLES] = 1.0f;
			}

			inline double getSplineDeceleration(float velocity) {
				return log(INFLEXION * fabs(velocity) / (mFlingFriction * mPhysicalCoeff));
			}

			inline int getSplineFlingDuration(float velocity) {
				double l = getSplineDeceleration(velocity);
				double decelMinusOne = DECELERATION_RATE - 1.0;
				return (int)(1000.0 * exp(l / decelMinusOne));
			}

			inline double getSplineFlingDistance(float velocity) {
				double l = getSplineDeceleration(velocity);
				double decelMinusOne = DECELERATION_RATE - 1.0;
				return mFlingFriction * mPhysicalCoeff * exp(DECELERATION_RATE / decelMinusOne * l);
			}
		public:
			DXScroller()
			{
				init();
				mFinished = true;
				mPpi = 4 * 160.0f;
				mDeceleration = computeDeceleration(0.015f);
				mFlywheel = true;
				mPhysicalCoeff = computeDeceleration(0.84f); // look and feel tuning
				mMode = -1;
			}

			/**
			 * The amount of friction applied to flings. The default value
			 * is {@link ViewConfiguration#getScrollFriction}.
			 *
			 * @param friction A scalar dimension-less value representing the coefficient of
			 *         friction.
			 */
			inline void setFriction(float friction)
			{
				mDeceleration = computeDeceleration(friction);
				mFlingFriction = friction;
			}

			inline float computeDeceleration(float friction)
			{
				return (9.80665f * 39.37f) * mPpi * friction;
			}

			/**
			 *
			 * Returns whether the scroller has finished scrolling.
			 *
			 * @return True if the scroller has finished scrolling, false otherwise.
			 */
			inline bool isFinished()
			{
				return mFinished;
			}

			/**
			 * Force the finished field to a particular value.
			 *
			 * @param finished The new finished value.
			 */
			inline void forceFinished(bool finished) {
				mFinished = finished;
			}

			/**
			 * Returns how long the scroll event will take, in milliseconds.
			 *
			 * @return The duration of the scroll in milliseconds.
			 */
			inline int getDuration() {
				return mDuration;
			}

			/**
			 * Returns the current X offset in the scroll.
			 *
			 * @return The new X offset as an absolute distance from the origin.
			 */
			inline int getCurrX() {
				return mCurrX;
			}

			/**
			 * Returns the current Y offset in the scroll.
			 *
			 * @return The new Y offset as an absolute distance from the origin.
			 */
			inline int getCurrY() {
				return mCurrY;
			}

			/**
			 * Returns the current velocity.
			 *
			 * @return The original velocity less the deceleration. Result may be
			 * negative.
			 */
			inline float getCurrVelocity() {
				return mMode == FLING_MODE ?
					mCurrVelocity : mVelocity - mDeceleration * timePassed() / 2000.0f;
			}

			/**
			 * Returns the start X offset in the scroll.
			 *
			 * @return The start X offset as an absolute distance from the origin.
			 */
			inline int getStartX() {
				return mStartX;
			}

			/**
			 * Returns the start Y offset in the scroll.
			 *
			 * @return The start Y offset as an absolute distance from the origin.
			 */
			inline int getStartY() {
				return mStartY;
			}

			/**
			 * Returns where the scroll will end. Valid only for "fling" scrolls.
			 *
			 * @return The final X offset as an absolute distance from the origin.
			 */
			inline int getFinalX() {
				return mFinalX;
			}

			/**
			 * Returns where the scroll will end. Valid only for "fling" scrolls.
			 *
			 * @return The final Y offset as an absolute distance from the origin.
			 */
			inline int getFinalY() {
				return mFinalY;
			}

			/**
			 * Call this when you want to know the new location.  If it returns true,
			 * the animation is not yet finished.
			 */
			inline bool update()
			{
				if (mFinished) return false;

				int timePassed = (int)(getTickCount() - mStartTime);

				if (timePassed < mDuration)
				{
					switch (mMode)
					{
					case SCROLL_MODE:
					{
						float x = getInterpolation(timePassed * mDurationReciprocal);
						mCurrX = mStartX + round(x * mDeltaX);
						mCurrY = mStartY + round(x * mDeltaY);
					}
					break;
					case FLING_MODE:
					{
						float t = (float)timePassed / mDuration;
						int index = (int)(NB_SAMPLES * t);
						float distanceCoef = 1.f;
						float velocityCoef = 0.f;
						if (index < NB_SAMPLES) {
							float t_inf = (float)index / NB_SAMPLES;
							float t_sup = (float)(index + 1) / NB_SAMPLES;
							float d_inf = SPLINE_POSITION[index];
							float d_sup = SPLINE_POSITION[index + 1];
							velocityCoef = (d_sup - d_inf) / (t_sup - t_inf);
							distanceCoef = d_inf + (t - t_inf) * velocityCoef;
						}

						mCurrVelocity = velocityCoef * mDistance / mDuration * 1000.0f;

						mCurrX = mStartX + round(distanceCoef * (mFinalX - mStartX));
						// Pin to mMinX <= mCurrX <= mMaxX
						mCurrX = __min(mCurrX, mMaxX);
						mCurrX = __max(mCurrX, mMinX);

						mCurrY = mStartY + round(distanceCoef * (mFinalY - mStartY));
						// Pin to mMinY <= mCurrY <= mMaxY
						mCurrY = __min(mCurrY, mMaxY);
						mCurrY = __max(mCurrY, mMinY);

						if (mCurrX == mFinalX && mCurrY == mFinalY)
							mFinished = true;
					}
					break;
					}
				}
				else {
					mCurrX = mFinalX;
					mCurrY = mFinalY;
					mFinished = true;
				}
				return true;
			}

			/**
			 * Start scrolling by providing a starting point and the distance to travel.
			 * The scroll will use the default value of 250 milliseconds for the
			 * duration.
			 *
			 * @param startX Starting horizontal scroll offset in pixels. Positive
			 *        numbers will scroll the content to the left.
			 * @param startY Starting vertical scroll offset in pixels. Positive numbers
			 *        will scroll the content up.
			 * @param dx Horizontal distance to travel. Positive numbers will scroll the
			 *        content to the left.
			 * @param dy Vertical distance to travel. Positive numbers will scroll the
			 *        content up.
			 */
			inline void startScroll(int startX, int startY, int dx, int dy) {
				startScroll(startX, startY, dx, dy, DEFAULT_DURATION);
			}

			/**
			 * Start scrolling by providing a starting point, the distance to travel,
			 * and the duration of the scroll.
			 *
			 * @param startX Starting horizontal scroll offset in pixels. Positive
			 *        numbers will scroll the content to the left.
			 * @param startY Starting vertical scroll offset in pixels. Positive numbers
			 *        will scroll the content up.
			 * @param dx Horizontal distance to travel. Positive numbers will scroll the
			 *        content to the left.
			 * @param dy Vertical distance to travel. Positive numbers will scroll the
			 *        content up.
			 * @param duration Duration of the scroll in milliseconds.
			 */
			inline void startScroll(int startX, int startY, int dx, int dy, int duration)
			{
				mMode = SCROLL_MODE;
				mFinished = false;
				mDuration = duration;
				mStartTime = getTickCount();
				mStartX = startX;
				mStartY = startY;
				mFinalX = startX + dx;
				mFinalY = startY + dy;
				mDeltaX = dx;
				mDeltaY = dy;
				mDurationReciprocal = 1.0f / (float)mDuration;
			}

			/**
			 * Start scrolling based on a fling gesture. The distance travelled will
			 * depend on the initial velocity of the fling.
			 *
			 * @param startX Starting point of the scroll (X)
			 * @param startY Starting point of the scroll (Y)
			 * @param velocityX Initial velocity of the fling (X) measured in pixels per
			 *        second.
			 * @param velocityY Initial velocity of the fling (Y) measured in pixels per
			 *        second
			 * @param minX Minimum X value. The scroller will not scroll past this
			 *        point.
			 * @param maxX Maximum X value. The scroller will not scroll past this
			 *        point.
			 * @param minY Minimum Y value. The scroller will not scroll past this
			 *        point.
			 * @param maxY Maximum Y value. The scroller will not scroll past this
			 *        point.
			 */
			inline void fling(int startX, int startY, int velocityX, int velocityY,
				int minX, int maxX, int minY, int maxY) {
				// Continue a scroll or fling in progress
				if (mFlywheel && !mFinished) {
					float oldVel = getCurrVelocity();

					float dx = (float)(mFinalX - mStartX);
					float dy = (float)(mFinalY - mStartY);
					float hyp = (float)hypot(dx, dy);

					float ndx = dx / hyp;
					float ndy = dy / hyp;

					float oldVelocityX = ndx * oldVel;
					float oldVelocityY = ndy * oldVel;
					if (signum(velocityX) == signum(oldVelocityX) &&
						signum(velocityY) == signum(oldVelocityY)) {
						velocityX += oldVelocityX;
						velocityY += oldVelocityY;
					}
				}

				mMode = FLING_MODE;
				mFinished = false;

				float velocity = (float)hypot(velocityX, velocityY);

				mVelocity = velocity;
				mDuration = getSplineFlingDuration(velocity);
				mStartTime = getTickCount();
				mStartX = startX;
				mStartY = startY;

				float coeffX = velocity == 0 ? 1.0f : velocityX / velocity;
				float coeffY = velocity == 0 ? 1.0f : velocityY / velocity;

				double totalDistance = getSplineFlingDistance(velocity);
				mDistance = (int)(totalDistance * signum(velocity));

				mMinX = minX;
				mMaxX = maxX;
				mMinY = minY;
				mMaxY = maxY;

				mFinalX = startX + (int)round(totalDistance * coeffX);
				// Pin to mMinX <= mFinalX <= mMaxX
				mFinalX = __min(mFinalX, mMaxX);
				mFinalX = __max(mFinalX, mMinX);

				mFinalY = startY + (int)round(totalDistance * coeffY);
				// Pin to mMinY <= mFinalY <= mMaxY
				mFinalY = __min(mFinalY, mMaxY);
				mFinalY = __max(mFinalY, mMinY);
			}

			/**
			 * Stops the animation. Contrary to {@link #forceFinished(boolean)},
			 * aborting the animating cause the scroller to move to the final x and y
			 * position
			 *
			 * @see #forceFinished(boolean)
			 */
			inline void abortAnimation() {
				mMode = -1;
				mCurrX = mFinalX;
				mCurrY = mFinalY;
				mFinished = true;
			}

			/**
			 * Extend the scroll animation. This allows a running animation to scroll
			 * further and longer, when used with {@link #setFinalX(int)} or {@link #setFinalY(int)}.
			 *
			 * @param extend Additional time to scroll in milliseconds.
			 * @see #setFinalX(int)
			 * @see #setFinalY(int)
			 */
			inline void extendDuration(int extend) {
				int passed = timePassed();
				mDuration = passed + extend;
				mDurationReciprocal = 1.0f / mDuration;
				mFinished = false;
			}

			/**
			 * Returns the time elapsed since the beginning of the scrolling.
			 *
			 * @return The elapsed time in milliseconds.
			 */
			inline int timePassed() {
				return (int)(getTickCount() - mStartTime);
			}

			/**
			 * Sets the final position (X) for this scroller.
			 *
			 * @param newX The new X offset as an absolute distance from the origin.
			 * @see #extendDuration(int)
			 * @see #setFinalY(int)
			 */
			inline void setFinalX(int newX) {
				mFinalX = newX;
				mDeltaX = mFinalX - mStartX;
				mFinished = false;
			}

			/**
			 * Sets the final position (Y) for this scroller.
			 *
			 * @param newY The new Y offset as an absolute distance from the origin.
			 * @see #extendDuration(int)
			 * @see #setFinalX(int)
			 */
			inline void setFinalY(int newY) {
				mFinalY = newY;
				mDeltaY = mFinalY - mStartY;
				mFinished = false;
			}

			/**
			 * @hide
			 */
			inline bool isScrollingInDirection(float xvel, float yvel) {
				return !mFinished && signum(xvel) == signum(mFinalX - mStartX) &&
					signum(yvel) == signum(mFinalY - mStartY);
			}
		};
	}
}
