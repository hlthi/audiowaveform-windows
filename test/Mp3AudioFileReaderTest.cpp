//------------------------------------------------------------------------------
//
// Copyright 2013-2017 BBC Research and Development
//
// Author: Chris Needham
//
// This file is part of Audio Waveform Image Generator.
//
// Audio Waveform Image Generator is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// Audio Waveform Image Generator is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Audio Waveform Image Generator.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include "Mp3AudioFileReader.h"
#include "mocks/MockAudioProcessor.h"
#include "util/Streams.h"

#include "gmock/gmock.h"

//------------------------------------------------------------------------------

using testing::_;
using testing::Eq;
using testing::InSequence;
using testing::Return;
using testing::StrEq;
using testing::StrictMock;
using testing::Test;

//------------------------------------------------------------------------------

class Mp3AudioFileReaderTest : public Test
{
    public:
        Mp3AudioFileReaderTest()
        {
        }

    protected:
        virtual void SetUp()
        {
            output.str(std::string());
            error.str(std::string());
        }

        virtual void TearDown()
        {
        }

        Mp3AudioFileReader reader_;
};

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldOpenMp3File)
{
    bool result = reader_.open("../test/data/test_file_stereo.mp3");

    ASSERT_TRUE(result);
    ASSERT_FALSE(output.str().empty());
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldReportErrorIfFileNotFound)
{
    bool result = reader_.open("../test/data/unknown.mp3");

    ASSERT_FALSE(result);
    ASSERT_FALSE(error.str().empty());
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldFailToProcessIfFileNotOpen)
{
    StrictMock<MockAudioProcessor> processor;

    EXPECT_CALL(processor, init(_, _, _)).Times(0);
    EXPECT_CALL(processor, process(_, _)).Times(0);
    EXPECT_CALL(processor, done()).Times(0);

    bool result = reader_.run(processor);
    ASSERT_FALSE(result);

    // No error message expected.
    ASSERT_TRUE(error.str().empty());
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldProcessStereoMp3File)
{
    bool result = reader_.open("../test/data/test_file_stereo.mp3");
    ASSERT_TRUE(result);

    StrictMock<MockAudioProcessor> processor;

    InSequence sequence; // Calls expected in the order listed below.

    EXPECT_CALL(processor, init(16000, 2, 8192)).WillOnce(Return(true));

    // TODO: Audacity reports length = 114624 samples (doesn't account for
    // decoding delay)
    // Total number of frames: 113519, 27 x 4096 frames then 1 x 2927
    EXPECT_CALL(processor, process(_, 4096)).Times(27).WillRepeatedly(Return(true));
    EXPECT_CALL(processor, process(_, 2927)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(processor, done());

    result = reader_.run(processor);

    std::string expected_output(
        "Input file: ../test/data/test_file_stereo.mp3\n"
        "Format: Audio MPEG layer III stream\n"
        "Bit rate: 128000 kbit/s\n"
        "CRC: no\n"
        "Mode: normal LR stereo\n"
        "Emphasis: no\n"
        "Sample rate: 16000 Hz\n"
        "Encoding delay: 1105\n"
        "Padding: 578\n"
        "\rDone: 0%"
        "\rDone: 42%"
        "\rDone: 78%"
        "\rDone: 100%\n"
        "Frames decoded: 199 (0:07.164)\n"
    );

    ASSERT_THAT(output.str(), StrEq(expected_output));
    ASSERT_TRUE(error.str().empty());
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldProcessMonoMp3File)
{
    bool result = reader_.open("../test/data/test_file_mono.mp3");
    ASSERT_TRUE(result);

    StrictMock<MockAudioProcessor> processor;

    InSequence sequence; // Calls expected in the order listed below.

    EXPECT_CALL(processor, init(16000, 1, 8192)).WillOnce(Return(true));

    // Total number of frames: 114095, which is 13 x 8192 frames then 1 x 7599
    EXPECT_CALL(processor, process(_, 8192)).Times(13).WillRepeatedly(Return(true));
    EXPECT_CALL(processor, process(_, 7599)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(processor, done());

    result = reader_.run(processor);

    std::string expected_output(
        "Input file: ../test/data/test_file_mono.mp3\n"
        "Format: Audio MPEG layer III stream\n"
        "Bit rate: 128000 kbit/s\n"
        "CRC: no\n"
        "Mode: single channel\n"
        "Emphasis: no\n"
        "Sample rate: 16000 Hz\n"
        "Encoding delay: 1105\n"
        "Padding: 576\n"
        "\rDone: 0%"
        "\rDone: 42%"
        "\rDone: 77%"
        "\rDone: 100%\n"
        "Frames decoded: 200 (0:07.200)\n"
    );

    ASSERT_THAT(output.str(), StrEq(expected_output));
    ASSERT_TRUE(error.str().empty());
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldProcessMp3FileWithId3Tags)
{
    bool result = reader_.open("../test/data/cl_T_01.mp3");
    ASSERT_TRUE(result);

    StrictMock<MockAudioProcessor> processor;

    InSequence sequence; // Calls expected in the order listed below.

    EXPECT_CALL(processor, init(44100, 1, 8192)).WillOnce(Return(true));

    // Total number of frames: 116352, which is 3 x 8192 frames then 1 x 6528
    EXPECT_CALL(processor, process(_, 8192)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(processor, process(_, 6528)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(processor, done());

    result = reader_.run(processor);

    std::string expected_output(
        "Input file: ../test/data/cl_T_01.mp3\n"
        "Format: Audio MPEG layer III stream\n"
        "Bit rate: 96000 kbit/s\n"
        "CRC: no\n"
        "Mode: single channel\n"
        "Emphasis: no\n"
        "Sample rate: 44100 Hz\n"
        "Encoding delay: unknown\n"
        "Padding: unknown\n"
        "\rDone: 0%"
        "\rDone: 100%\n"
        "Frames decoded: 27 (0:00.705)\n"
    );

    ASSERT_THAT(output.str(), StrEq(expected_output));
    ASSERT_TRUE(error.str().empty());
}

//------------------------------------------------------------------------------

// An audio processor that looks for the first frame with non-zero sample
// values.

class DecodingDelayDetector : public AudioProcessor
{
    public:
        DecodingDelayDetector() :
            channels_(0),
            frame_count_(0),
            start_frame_(-1)
        {
        }

    public:
        virtual bool init(
            int /* sample_rate */,
            int channels,
            int /* buffer_size */)
        {
            channels_ = channels;
            return true;
        }

        virtual bool process(
            const short* input_buffer,
            int input_frame_count)
        {
            if (start_frame_ != -1) {
                return true;
            }

            for (int i = 0; i < input_frame_count * channels_; ++i) {
                if (input_buffer[i] != 0) {
                    start_frame_ = frame_count_ + i;
                    break;
                }
            }

            frame_count_ += input_frame_count;

            return true;
        }

        virtual void done()
        {
        }

    public:
        int getStartFrame() const
        {
            return start_frame_;
        }

    private:
        int channels_;
        int frame_count_;
        int start_frame_;
};

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldAccountForDecodingDelay)
{
    bool result = reader_.open("../test/data/test_file_stereo.mp3");
    ASSERT_TRUE(result);

    DecodingDelayDetector processor;

    result = reader_.run(processor);
    ASSERT_TRUE(result);

    ASSERT_THAT(processor.getStartFrame(), Eq(0));
}

//------------------------------------------------------------------------------

TEST_F(Mp3AudioFileReaderTest, shouldNotProcessFileMoreThanOnce)
{
    bool result = reader_.open("../test/data/test_file_mono.mp3");
    ASSERT_TRUE(result);

    StrictMock<MockAudioProcessor> processor;

    InSequence sequence; // Calls expected in the order listed below.

    EXPECT_CALL(processor, init(16000, 1, 8192)).WillOnce(Return(true));

    // Total number of frames: 114095, which is 13 x 8192 frames then 1 x 7599
    EXPECT_CALL(processor, process(_, 8192)).Times(13).WillRepeatedly(Return(true));
    EXPECT_CALL(processor, process(_, 7599)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(processor, done());

    result = reader_.run(processor);

    // Attempting to process the file again should fail
    output.str(std::string());

    result = reader_.run(processor);
    ASSERT_FALSE(result);

    ASSERT_TRUE(output.str().empty());
    ASSERT_TRUE(error.str().empty());
}

//------------------------------------------------------------------------------
/*
TEST_F(Mp3AudioFileReaderTest, shouldReportErrorIfNotAnMp3File)
{
    bool result = reader_.open("../test/data/test_file_stereo.wav");

    ASSERT_TRUE(result);

    TestAudioProcessor processor;
    result = reader_.run(processor);

    // ASSERT_FALSE(result);

    ASSERT_FALSE(error.str().empty());
    ASSERT_EQ("", error.str());
}
*/
//------------------------------------------------------------------------------
