#include "file_player_editor.h"
#include "file_player_plugin.h"

namespace file_player::editor
{
File_Player_Editor::File_Player_Editor (plugin::File_Player_Plugin& plug)
    : plugin { plug }
{
    audio_format_manager.registerBasicFormats();

    select_file_button.onClick = [this]
    {
        file_chooser = std::make_shared<juce::FileChooser> ("Select audio file...",
                                                            juce::File {},
                                                            "*.wav;*.mp3;*.flac");
        static constexpr auto file_chooser_flags = juce::FileBrowserComponent::canSelectFiles;
        file_chooser->launchAsync (file_chooser_flags,
                                   [this] (const juce::FileChooser& fc)
                                   {
                                       if (fc.getResults().isEmpty())
                                           return;

                                       const auto file_to_play = fc.getResult();
                                       auto&& reader = std::unique_ptr<juce::AudioFormatReader> (audio_format_manager.createReaderFor (file_to_play));
                                       if (reader == nullptr)
                                       {
                                           jassertfalse;
                                           return;
                                       }

                                       //                                       const auto sample_rate = reader->sampleRate; // @TODO: actually use this!
                                       const auto num_channels = (int) reader->numChannels;
                                       const auto num_samples = (int) reader->lengthInSamples;
                                       auto buffer = std::make_unique<juce::AudioBuffer<float>> (num_channels, num_samples);

                                       if (! reader->read (buffer->getArrayOfWritePointers(), num_channels, 0, num_samples))
                                       {
                                           jassertfalse; // there was some problem reading the samples from the audio file into the buffer!
                                           return;
                                       }

                                       plugin.buffer_life_queue.enqueue (std::move (buffer));
                                   });
    };
    addAndMakeVisible (select_file_button);

    setSize (400, 400);
}

void File_Player_Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void File_Player_Editor::resized()
{
    select_file_button.setBounds (juce::Rectangle { 80, 35 }.withCentre (getLocalBounds().getCentre()));
}
} // namespace file_player::editor
