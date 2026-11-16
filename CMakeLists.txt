cmake_minimum_required(VERSION 3.22)
project(GemS2TurboVST VERSION 0.6.0)

# Build with: cmake -S . -B build -DJUCE_PATH=/path/to/JUCE
if(NOT JUCE_PATH)
  message(FATAL_ERROR "Set JUCE_PATH to a JUCE checkout (JUCE 7 or later).")
endif()
add_subdirectory(${JUCE_PATH} JUCE)
juce_add_plugin(GemS2TurboVST
  COMPANY_NAME "Independent"
  PLUGIN_MANUFACTURER_CODE Idgm
  PLUGIN_CODE Gs2t
  FORMATS VST3 Standalone
  PRODUCT_NAME "GEM S2 Turbo"
  IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE)
target_sources(GemS2TurboVST PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(GemS2TurboVST PRIVATE cxx_std_17)
juce_generate_juce_header(GemS2TurboVST)
target_compile_definitions(GemS2TurboVST PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(GemS2TurboVST PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

