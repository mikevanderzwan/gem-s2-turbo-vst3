#include "PluginProcessor.h"

class GemS2Editor final : public juce::AudioProcessorEditor
{
public:
    explicit GemS2Editor (GemS2Processor& p)
        : AudioProcessorEditor (p), processor (p), state (p.state),
          keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        setSize (1660, 940);
        setResizable (true, true);
        setResizeLimits (1420, 804, 1900, 1076);
        getConstrainer()->setFixedAspectRatio (1660.0 / 940.0);

        addAndMakeVisible (importButton);
        importButton.setButtonText ("LOAD SAMPLE / FILE");
        importButton.setColour (juce::TextButton::buttonColourId, colour (0xffd34e2f));
        importButton.setColour (juce::TextButton::textColourOffId, colour (0xff181818));
        importButton.setColour (juce::TextButton::textColourOnId, colour (0xff181818));
        importButton.onClick = [this] { openFile(); };

        addAndMakeVisible (fileInfo);
        fileInfo.setMultiLine (true);
        fileInfo.setReadOnly (true);
        fileInfo.setScrollbarsShown (true);
        fileInfo.setColour (juce::TextEditor::backgroundColourId, colour (0xff111313));
        fileInfo.setColour (juce::TextEditor::textColourId, colour (0xffe4e3dd));
        fileInfo.setColour (juce::TextEditor::outlineColourId, colour (0xff77716a));
        fileInfo.setFont (juce::Font (juce::FontOptions (12.0f)));
        fileInfo.setText ("WAV / AIFF / FLAC  Load as a sample\n.SYX  Extract recognized GEM packets\n.IMG  Inspect a FAT12 disk image", juce::dontSendNotification);

        // Layer controls
        addKnob ("LEVEL", "osc1level", 268, 228, 72);
        addKnob ("LEVEL", "osc2level", 30, 421, 58);
        addKnob ("TRANSPOSE", "osc2semi", 139, 421, 58);
        addKnob ("DETUNE", "detune", 248, 421, 58);

        // Filters and amp envelope
        addKnob ("CUTOFF", "f1cut", 404, 229, 58);
        addKnob ("RESONANCE", "f1res", 513, 229, 58);
        addChoice ("TYPE", "f1type", 399, 354, 212);
        addKnob ("CUTOFF", "f2cut", 658, 229, 58);
        addKnob ("RESONANCE", "f2res", 767, 229, 58);
        addChoice ("TYPE", "f2type", 653, 354, 212);
        addKnob ("ATTACK", "attack", 901, 235, 44);
        addKnob ("DECAY", "decay", 966, 235, 44);
        addKnob ("SUSTAIN", "sustain", 1031, 235, 44);
        addKnob ("RELEASE", "release", 1096, 235, 44);

        // Effects, output and modulation
        addKnob ("MIX", "reverb", 464, 449, 58);
        addKnob ("MIX", "chorus", 712, 449, 58);
        addKnob ("MASTER", "master", 909, 432, 58);
        addKnob ("PAN", "pan", 1058, 432, 58);
        addKnob ("RATE", "lforate", 1284, 143, 62);
        addKnob ("DEPTH", "lfodepth", 1466, 143, 62);
        addRoute ("LAYER A TO DRY OUTPUT", "dryA", 1208, 386, 185);
        addRoute ("LAYER B TO DRY OUTPUT", "dryB", 1420, 386, 185);

        // Seven performance sliders mirror useful sound controls.
        addMacro ("F1 CUT", "f1cut", 184, 786);
        addMacro ("F2 CUT", "f2cut", 228, 786);
        addMacro ("RESON", "f1res", 272, 786);
        addMacro ("ATTACK", "attack", 316, 786);
        addMacro ("LFO", "lforate", 360, 786);
        addMacro ("REVERB", "reverb", 404, 786);
        addMacro ("PAN", "pan", 448, 786);

        pitchWheel.setRange (0.0, 16383.0, 1.0);
        pitchWheel.setSliderStyle (juce::Slider::LinearVertical);
        pitchWheel.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        pitchWheel.setColour (juce::Slider::thumbColourId, colour (0xffffbc50));
        pitchWheel.setValue (8192.0, juce::dontSendNotification);
        pitchWheel.onValueChange = [this] { processor.setPitchWheel ((int) pitchWheel.getValue()); };
        addAndMakeVisible (pitchWheel);

        modWheel.setRange (0.0, 127.0, 1.0);
        modWheel.setSliderStyle (juce::Slider::LinearVertical);
        modWheel.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        modWheel.setColour (juce::Slider::thumbColourId, colour (0xffffbc50));
        modWheel.setValue (0.0, juce::dontSendNotification);
        modWheel.onValueChange = [this] { processor.setModWheel ((int) modWheel.getValue()); };
        addAndMakeVisible (modWheel);

        addAndMakeVisible (keyboard);
        keyboard.setKeyWidth (17.6f);
        keyboard.setAvailableRange (24, 84);
        keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, colour (0x77ed7b36));
        keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, colour (0x44f5b65d));
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (colour (0xff101213));
        g.addTransform (juce::AffineTransform::scale ((float) getWidth() / 1660.0f,
                                                      (float) getHeight() / 940.0f));

        // Brushed metal fascia and header.
        g.setColour (colour (0xff292b2b));
        g.fillRect (0, 0, 1660, 72);
        g.setColour (colour (0xffd84c2f));
        g.fillRect (0, 70, 1660, 4);
        g.setColour (colour (0xffe5e1d8));
        g.setFont (juce::Font (juce::FontOptions (36.0f).withStyle ("Bold")));
        g.drawText ("GEM", 24, 10, 132, 48, juce::Justification::centredLeft);
        g.setFont (juce::Font (juce::FontOptions (37.0f).withStyle ("Bold Italic")));
        g.setColour (colour (0xfff16a43));
        g.drawText ("S2 TURBO", 157, 9, 315, 50, juce::Justification::centredLeft);

        g.setColour (colour (0xffce8734));
        g.fillRoundedRectangle (520.0f, 12.0f, 575.0f, 46.0f, 4.0f);
        g.setColour (colour (0xff271b12));
        g.fillRoundedRectangle (525.0f, 16.0f, 565.0f, 38.0f, 3.0f);
        g.setColour (colour (0xffffc663));
        g.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
        g.drawText ("GEM S2 TURBO  |  SOUND EDITOR  |  BUILD 0.6", 538, 20, 540, 28,
                    juce::Justification::centred);
        g.setColour (colour (0xffffbc50));
        g.fillRoundedRectangle (1130.0f, 17.0f, 130.0f, 35.0f, 4.0f);
        g.setColour (colour (0xff21170d));
        g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
        g.drawText ("32 VOICES", 1130, 24, 130, 20, juce::Justification::centred);
        g.setColour (colour (0xff242829));
        g.fillRoundedRectangle (1274.0f, 17.0f, 128.0f, 35.0f, 4.0f);
        g.setColour (colour (0xffe95a38));
        g.fillRoundedRectangle (1278.0f, 21.0f, 120.0f, 27.0f, 3.0f);
        g.setColour (colour (0xff241710));
        g.drawText ("SOUND", 1278, 25, 120, 18, juce::Justification::centred);
        g.setColour (colour (0xffddd9d1));
        g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
        g.drawText ("READY  |  32 VOICES", 1420, 24, 215, 22, juce::Justification::centredRight);

        panel (g, 14, 82, 360, 114, "WAVESET  |  WAVEFORM / MULTISAMPLE");
        panel (g, 14, 204, 360, 152, "LAYER A");
        panel (g, 14, 364, 360, 184, "LAYER B");
        panel (g, 384, 82, 790, 112, "SIGNAL FLOW  |  SERIAL");
        panel (g, 384, 204, 246, 210, "FILTER 1  |  12 dB");
        panel (g, 638, 204, 246, 210, "FILTER 2  |  12 dB");
        panel (g, 892, 204, 282, 210, "AMP ENVELOPE");
        panel (g, 384, 422, 246, 126, "DSP 1  |  REVERB");
        panel (g, 638, 422, 246, 126, "DSP 2  |  MODULATION");
        panel (g, 892, 422, 282, 126, "OUTPUT");
        panel (g, 1184, 82, 462, 226, "LFO  |  PITCH MODULATION");
        panel (g, 1184, 318, 462, 230, "ASSIGNABLE DRY OUTPUTS");
        panel (g, 14, 562, 600, 165, "ASSIGNABLE PERFORMANCE SLIDERS  |  1-7");
        panel (g, 624, 562, 1022, 165, "SAMPLE / SYSEX / DISK IMAGE");
        panel (g, 14, 740, 1632, 188, "PITCH  |  MODULATION  |  61-KEY PERFORMANCE KEYBOARD");

        drawWave (g, 34, 122, 132, 50);
        g.setColour (colour (0xffffc663));
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText ("OSCILLATORS 1 + 2", 188, 127, 164, 20, juce::Justification::centredLeft);
        g.setColour (colour (0xffb4b6b1));
        g.setFont (juce::Font (juce::FontOptions (10.5f)));
        g.drawText ("Sine source; imported WAV sample can replace it", 188, 149, 171, 32,
                    juce::Justification::centredLeft);
        g.drawText ("OSC 1 / SAMPLE", 36, 250, 200, 22, juce::Justification::centredLeft);
        g.drawText ("BASE LAYER", 36, 276, 180, 18, juce::Justification::centredLeft);
        g.drawText ("OSC 2", 35, 394, 180, 18, juce::Justification::centredLeft);

        flowBlock (g, 403, 119, 142, 47, "LAYERS A / B");
        flowBlock (g, 570, 119, 118, 47, "FILTER 1");
        flowBlock (g, 713, 119, 118, 47, "FILTER 2");
        flowBlock (g, 856, 119, 95, 47, "AMP");
        flowBlock (g, 976, 119, 95, 47, "PAN");
        g.setColour (colour (0xffffb84d));
        g.setFont (juce::Font (juce::FontOptions (21.0f).withStyle ("Bold")));
        g.drawText (">", 545, 128, 23, 28, juce::Justification::centred);
        g.drawText (">", 689, 128, 23, 28, juce::Justification::centred);
        g.drawText (">", 832, 128, 23, 28, juce::Justification::centred);
        g.drawText (">", 952, 128, 23, 28, juce::Justification::centred);

        drawEnvelope (g, 906, 335, 254, 62, processor.param ("attack"), processor.param ("decay"),
                     processor.param ("sustain"), processor.param ("release"));
        g.setColour (colour (0xffd8d4ca));
        g.setFont (juce::Font (juce::FontOptions (10.0f)));
        g.drawText ("ADSR", 906, 397, 254, 13, juce::Justification::centred);
        g.drawText ("WET STEREO", 898, 526, 270, 16, juce::Justification::centred);

        g.setColour (colour (0xffc2c4c0));
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        g.drawText ("PITCH", 22, 763, 58, 18, juce::Justification::centred);
        g.drawText ("MOD", 86, 763, 58, 18, juce::Justification::centred);
    }

    void resized() override
    {
        const auto sx = (float) getWidth() / 1660.0f;
        const auto sy = (float) getHeight() / 940.0f;
        auto bounds = [sx, sy] (int x, int y, int w, int h)
        {
            return juce::Rectangle<int> ((int) std::round (x * sx), (int) std::round (y * sy),
                                         (int) std::round (w * sx), (int) std::round (h * sy));
        };
        importButton.setBounds (bounds (646, 609, 220, 38));
        fileInfo.setBounds (bounds (883, 600, 737, 108));
        pitchWheel.setBounds (bounds (30, 786, 34, 124));
        modWheel.setBounds (bounds (92, 786, 34, 124));
        keyboard.setBounds (bounds (540, 783, 1083, 129));
    }

private:
    GemS2Processor& processor;
    juce::AudioProcessorValueTreeState& state;
    juce::MidiKeyboardComponent keyboard;
    juce::TextButton importButton;
    juce::TextEditor fileInfo;
    juce::Slider pitchWheel, modWheel;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<std::unique_ptr<juce::Slider>> knobs, macros;
    std::vector<std::unique_ptr<juce::Label>> labels, macroLabels;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> links, macroLinks;
    std::vector<std::unique_ptr<juce::ComboBox>> choices;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choiceLinks;

    static juce::Colour colour (juce::uint32 c) { return juce::Colour (c); }

    void openFile()
    {
        chooser = std::make_unique<juce::FileChooser> ("Open sample, SysEx file, or floppy image", juce::File{},
                                                        "*.wav;*.aiff;*.aif;*.flac;*.syx;*.img");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
        {
            const auto file = fc.getResult();
            if (! file.existsAsFile())
                return;

            if (file.hasFileExtension ("wav;aif;aiff;flac;ogg"))
                processor.loadSampleFile (file);
            else if (file.hasFileExtension ("img"))
                processor.inspectDiskImage (file);
            else
                processor.importSysEx (file);

            fileInfo.setText (processor.getImportStatus(), juce::dontSendNotification);
        });
    }

    void panel (juce::Graphics& g, int x, int y, int w, int h, const juce::String& title)
    {
        g.setColour (colour (0xff202323));
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 5.0f);
        g.setColour (colour (0xff77716a));
        g.drawRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 5.0f, 1.0f);
        g.setColour (colour (0xffdf512f));
        g.fillRect (x + 11, y + 10, 5, 13);
        g.setColour (colour (0xfff0ede6));
        g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
        g.drawText (title, x + 24, y + 7, w - 34, 20, juce::Justification::centredLeft);
        g.setColour (colour (0xff744c3c));
        g.fillRect (x + 12, y + 31, w - 24, 1);
    }

    void flowBlock (juce::Graphics& g, int x, int y, int w, int h, const juce::String& text)
    {
        g.setColour (colour (0xff292d2d));
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 4.0f);
        g.setColour (colour (0xffc7c1b8));
        g.drawRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 4.0f, 1.2f);
        g.setColour (colour (0xfff3eee5));
        g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
        g.drawText (text, x + 3, y + 4, w - 6, h - 8, juce::Justification::centred);
    }

    void addKnob (const char* title, const char* id, int x, int y, int size)
    {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, size + 14, 17);
        slider->setColour (juce::Slider::rotarySliderFillColourId, colour (0xffe7673e));
        slider->setColour (juce::Slider::thumbColourId, colour (0xffffbd54));
        slider->setColour (juce::Slider::textBoxTextColourId, colour (0xffffe4ad));
        slider->setColour (juce::Slider::textBoxBackgroundColourId, colour (0xff171919));
        addAndMakeVisible (*slider);
        slider->setBounds (x, y + 16, size + 14, size + 34);

        auto label = std::make_unique<juce::Label>();
        label->setText (title, juce::dontSendNotification);
        label->setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        label->setColour (juce::Label::textColourId, colour (0xffe1ddd4));
        label->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (*label);
        label->setBounds (x - 4, y, size + 22, 15);

        links.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, id, *slider));
        knobs.push_back (std::move (slider));
        labels.push_back (std::move (label));
    }

    void addChoice (const char* title, const char* id, int x, int y, int w)
    {
        auto label = std::make_unique<juce::Label>();
        label->setText (title, juce::dontSendNotification);
        label->setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        label->setColour (juce::Label::textColourId, colour (0xffe1ddd4));
        addAndMakeVisible (*label);
        label->setBounds (x, y, w, 14);
        labels.push_back (std::move (label));

        auto combo = std::make_unique<juce::ComboBox>();
        combo->addItemList ({ "Low Pass", "High Pass", "Band Pass", "Parametric Cut", "Parametric Boost" }, 1);
        combo->setColour (juce::ComboBox::backgroundColourId, colour (0xff111313));
        combo->setColour (juce::ComboBox::textColourId, colour (0xfff2eee5));
        combo->setColour (juce::ComboBox::outlineColourId, colour (0xff817a71));
        addAndMakeVisible (*combo);
        combo->setBounds (x, y + 15, w, 25);
        choiceLinks.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, id, *combo));
        choices.push_back (std::move (combo));
    }

    void addRoute (const char* title, const char* id, int x, int y, int w)
    {
        auto label = std::make_unique<juce::Label>();
        label->setText (title, juce::dontSendNotification);
        label->setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        label->setColour (juce::Label::textColourId, colour (0xffe1ddd4));
        addAndMakeVisible (*label);
        label->setBounds (x, y, w, 15);
        labels.push_back (std::move (label));

        auto combo = std::make_unique<juce::ComboBox>();
        combo->addItemList ({ "Out 1", "Out 2", "Out 3", "Out 4" }, 1);
        combo->setColour (juce::ComboBox::backgroundColourId, colour (0xff111313));
        combo->setColour (juce::ComboBox::textColourId, colour (0xfff2eee5));
        combo->setColour (juce::ComboBox::outlineColourId, colour (0xff817a71));
        addAndMakeVisible (*combo);
        combo->setBounds (x, y + 16, w, 28);
        choiceLinks.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, id, *combo));
        choices.push_back (std::move (combo));
    }

    void addMacro (const char* title, const char* id, int x, int y)
    {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle (juce::Slider::LinearVertical);
        slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider->setColour (juce::Slider::thumbColourId, colour (0xffffbd54));
        slider->setColour (juce::Slider::trackColourId, colour (0xffdf643e));
        addAndMakeVisible (*slider);
        slider->setBounds (x, y, 30, 104);

        auto label = std::make_unique<juce::Label>();
        label->setText (title, juce::dontSendNotification);
        label->setFont (juce::Font (juce::FontOptions (8.5f).withStyle ("Bold")));
        label->setColour (juce::Label::textColourId, colour (0xffe4dfd4));
        label->setJustificationType (juce::Justification::centred);
        addAndMakeVisible (*label);
        label->setBounds (x - 10, y - 20, 50, 16);
        macroLinks.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, id, *slider));
        macros.push_back (std::move (slider));
        macroLabels.push_back (std::move (label));
    }

    void drawWave (juce::Graphics& g, int x, int y, int w, int h)
    {
        g.setColour (colour (0xff111414));
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 3.0f);
        juce::Path wave;
        wave.startNewSubPath ((float) x + 3, (float) y + h * 0.5f);
        for (int i = 1; i < w - 4; ++i)
            wave.lineTo ((float) x + i, (float) y + h * (0.5f - 0.36f * std::sin (i * 0.075f)));
        g.setColour (colour (0xffffb84d));
        g.strokePath (wave, juce::PathStrokeType (1.5f));
    }

    void drawEnvelope (juce::Graphics& g, int x, int y, int w, int h,
                       float attack, float decay, float sustain, float release)
    {
        g.setColour (colour (0xff111414));
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, 3.0f);
        g.setColour (colour (0xff343a38));
        for (int i = 1; i < 8; ++i)
            g.drawVerticalLine (x + i * w / 8, (float) y, (float) y + h);
        for (int i = 1; i < 3; ++i)
            g.drawHorizontalLine (y + i * h / 3, (float) x, (float) x + w);

        auto normTime = [] (float v) { return juce::jlimit (0.08f, 1.0f, (std::log10 (1.0f + v * 9.0f)) / 1.0f); };
        float a = normTime (attack), d = normTime (decay), r = normTime (release);
        float usable = (float) w - 12.0f;
        float total = a + d + 0.9f + r;
        float start = (float) x + 6.0f;
        float xa = start + usable * a / total;
        float xd = xa + usable * d / total;
        float xs = xd + usable * 0.9f / total;
        juce::Path path;
        path.startNewSubPath (start, (float) y + h - 5.0f);
        path.lineTo (xa, (float) y + 5.0f);
        path.lineTo (xd, (float) y + h * (1.0f - juce::jlimit (0.0f, 1.0f, sustain)));
        path.lineTo (xs, (float) y + h * (1.0f - juce::jlimit (0.0f, 1.0f, sustain)));
        path.lineTo ((float) x + w - 6.0f, (float) y + h - 5.0f);
        g.setColour (colour (0xffffb84d));
        g.strokePath (path, juce::PathStrokeType (2.0f));
    }
};

juce::AudioProcessorEditor* GemS2Processor::createEditor()
{
    return new GemS2Editor (*this);
}

