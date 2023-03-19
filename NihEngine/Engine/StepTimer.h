#pragma once

namespace DX
{
    class StepTimer
    {
    public:
        StepTimer()
            : m_elapsedTicks(0)
            , m_totalTicks(0)
            , m_leftOverTicks(0)
            , m_frameCount(0)
            , m_framesPerSecond(0)
            , m_framesThisSecond(0)
            , m_qpcSecondCounter(0)
            , m_isFixedTimeStep(false)
            , m_targetElapsedTicks(0)
        {
            if (!QueryPerformanceFrequency(&m_qpcFrequency) || !QueryPerformanceCounter(&m_qpcLastTime))
            {
                // we should assert here if we can't initialize
            }

            m_qpcMaxDelta = static_cast<uint64_t>(m_qpcFrequency.QuadPart / 10);
        }

        uint64_t GetElapsedTicks() const { return m_elapsedTicks; }
        double GetElapsedSeconds() const { return TicksToSeconds(m_elapsedTicks); }

        uint64_t GetTotalTicks() const { return m_totalTicks; }
        double GetTotalSeconds() const { return TicksToSeconds(m_totalTicks); }

        uint32_t GetFrameCount() const { return m_frameCount; }

        uint32_t GetFramesPerSecond() const { return m_framesPerSecond; }

        void SetFixedTimeStep(bool isFixedTimeStep) { m_isFixedTimeStep = isFixedTimeStep; }

        void SetTargetElapsedTicks(uint64_t targetElapsed) { m_targetElapsedTicks = targetElapsed; }
        void SetTargetElapsedSeconds(double targetElapsed) { m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

        static constexpr uint64_t TicksPerSeconds = 10'000'000;
        static constexpr double TicksToSeconds(uint64_t ticks) { return static_cast<double>(ticks) / TicksPerSeconds; }
        static constexpr uint64_t SecondsToTicks(double seconds) { return static_cast<uint64_t>(seconds * TicksPerSeconds); }

        void ResetElapsedTime()
        {
            if (!QueryPerformanceCounter(&m_qpcLastTime))
            {
                // we should assert here
            }

            m_leftOverTicks = 0;
            m_framesPerSecond = 0;
            m_framesThisSecond = 0;
            m_qpcSecondCounter = 0;
        }

        template<typename TUpdate>
        void Tick(const TUpdate& update)
        {
            LARGE_INTEGER currentTime;
            if (!QueryPerformanceCounter(&currentTime))
            {
                // we should assert here
            }

            uint64_t deltaTime = static_cast<uint64_t>(currentTime.QuadPart - m_qpcLastTime.QuadPart);

            m_qpcLastTime = currentTime;
            m_qpcSecondCounter = deltaTime;

            if (deltaTime > m_qpcMaxDelta)
            {
                deltaTime = m_qpcMaxDelta;
            }

            const uint32_t lastFrameCount = m_frameCount;

            if (m_isFixedTimeStep)
            {
                // Fixed timestep update logic

                // If the engine is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
                // the clock to exactly match the target value. This prevents tiny and irrelevant errors
                // from accumulating over time. Without this clamping, a game that requested a 60 fps
                // fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
                // accumulate enough tiny errors that it would drop a frame. It is better to just round
                // small deviations down to zero to leave things  running smoothly

                if (static_cast<uint64_t>(std::abs(static_cast<int64_t>(deltaTime - m_targetElapsedTicks))) < TicksPerSeconds / 4000)
                {
                    deltaTime = m_targetElapsedTicks;
                }

                m_leftOverTicks += deltaTime;

                while (m_leftOverTicks >= m_targetElapsedTicks)
                {
                    m_elapsedTicks = m_targetElapsedTicks;
                    m_totalTicks += m_targetElapsedTicks;
                    m_leftOverTicks -= m_targetElapsedTicks;
                    m_frameCount++;

                    update();
                }
            }
            else
            {
                // variable timestep update logic
                m_elapsedTicks = deltaTime;
                m_totalTicks += deltaTime;
                m_leftOverTicks = 0;
                m_frameCount++;

                update();
            }

            // Track the current framerate
            if (m_frameCount != lastFrameCount)
            {
                m_framesThisSecond++;
            }

            if (m_qpcSecondCounter >= static_cast<uint64_t>(m_qpcFrequency.QuadPart))
            {
                m_framesPerSecond = m_framesThisSecond;
                m_framesThisSecond = 0;
                m_qpcSecondCounter %= static_cast<uint64_t>(m_qpcFrequency.QuadPart);
            }
        }
    private:
        LARGE_INTEGER m_qpcFrequency;
        LARGE_INTEGER m_qpcLastTime;
        uint64_t m_qpcMaxDelta;

        uint64_t m_elapsedTicks;
        uint64_t m_totalTicks;
        uint64_t m_leftOverTicks;

        uint32_t m_frameCount;
        uint32_t m_framesPerSecond;
        uint32_t m_framesThisSecond;
        uint64_t m_qpcSecondCounter;

        bool m_isFixedTimeStep;
        uint64_t m_targetElapsedTicks;
    };
}