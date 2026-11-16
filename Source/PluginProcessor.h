#pragma once
#include <JuceHeader.h>
#include <memory>

class GemS2Processor final : public juce::AudioProcessor {
    class Voice;
    class SynthSound;
public:
    GemS2Processor();
    const juce::String getName() const override { return "GEM S2 Turbo"; }
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 128; }
    int getCurrentProgram() override { return program; }
    void setCurrentProgram(int p) override { program = juce::jlimit(0,127,p); }
    const juce::String getProgramName(int) override { return "Init Sound"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState state;
    juce::MidiKeyboardState keyboardState;
    float param(const char* id) const;
    void loadSampleFile(const juce::File& file);
    bool importSysEx(const juce::File& file);
    juce::String inspectDiskImage(const juce::File& file);
    juce::String getImportStatus() const { return importStatus; }
    void setPitchWheel(int value);
    void setModWheel(int value);
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    juce::Synthesiser synth;
    juce::AudioFormatManager formats;
    juce::String importStatus { "No sample loaded" };
    juce::AudioBuffer<float> scratch;
    juce::Reverb reverb;
    juce::dsp::Chorus<float> chorus;
    std::shared_ptr<const juce::AudioBuffer<float>> sampleData;
    double importedSampleRate=44100.0;
    int program=0;
};

