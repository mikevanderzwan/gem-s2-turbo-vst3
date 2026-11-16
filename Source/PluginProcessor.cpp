#include "PluginProcessor.h"

class GemS2Processor::SynthSound final : public juce::SynthesiserSound {
public: bool appliesToNote(int) override{return true;} bool appliesToChannel(int) override{return true;}
};
class GemS2Processor::Voice final : public juce::SynthesiserVoice {
public:
 explicit Voice(GemS2Processor& p):owner(p){env.setSampleRate(44100.0);}
 bool canPlaySound(juce::SynthesiserSound* s) override{return dynamic_cast<SynthSound*>(s)!=nullptr;}
 void startNote(int n,float vel,juce::SynthesiserSound*,int) override {note=n;velocity=vel;phaseA=phaseB=samplePos=0;sample=std::atomic_load(&owner.sampleData);env.setParameters({owner.param("attack"),owner.param("decay"),owner.param("sustain"),owner.param("release")});env.noteOn();}
 void stopNote(float,bool tail) override {env.noteOff();if(!tail)clearCurrentNote();}
 void pitchWheelMoved(int value) override{pitchBend=(value-8192)/8192.f*2.f;}
 void controllerMoved(int number,int value) override{if(number==1)modWheel=value/127.f;}
 void prepare(double sr){env.setSampleRate(sr);juce::dsp::ProcessSpec sp{sr,512,1};f1.prepare(sp);f2.prepare(sp);f1.reset();f2.reset();}
 void renderNextBlock(juce::AudioBuffer<float>& out,int start,int count) override {
  const float sr=(float)owner.getSampleRate(); if(sr<=0)return;
  const float hz=(float)juce::MidiMessage::getMidiNoteInHertz(note)*std::pow(2.f,pitchBend/12.f);
  const float a=owner.param("osc1level"),b=owner.param("osc2level"),semi=owner.param("osc2semi"),det=owner.param("detune");
  auto configure=[&](juce::dsp::StateVariableTPTFilter<float>& f,const char* cut,const char* res,const char* type){f.setCutoffFrequency(juce::jlimit(20.f,20000.f,owner.param(cut)));f.setResonance(juce::jlimit(.1f,10.f,.7f+owner.param(res)*8.f));int t=(int)owner.param(type);using T=juce::dsp::StateVariableTPTFilterType;f.setType(t==1?T::highpass:t>=2?T::bandpass:T::lowpass);};configure(f1,"f1cut","f1res","f1type");configure(f2,"f2cut","f2res","f2type");
  const float mod=owner.param("lfodepth")+modWheel*.7f; const float rate=owner.param("lforate");
  const float pan=juce::jlimit(-1.f,1.f,owner.param("pan"));const float gl=std::cos((pan+1.f)*juce::MathConstants<float>::pi*.25f),gr=std::sin((pan+1.f)*juce::MathConstants<float>::pi*.25f);
  // The wet stereo output is always the first two channels of the host's
  // process buffer. Write it directly so a host with disabled auxiliary
  // buses cannot accidentally leave the instrument silent.
  const int routeA=(int)owner.param("dryA")+1,routeB=(int)owner.param("dryB")+1;auto dryA=owner.getBusBuffer(out,false,routeA);auto dryB=owner.getBusBuffer(out,false,routeB);
  for(int i=0;i<count;++i){float lfo=std::sin(juce::MathConstants<float>::twoPi*lfoPhase);lfoPhase+=rate/sr;if(lfoPhase>=1)lfoPhase-=1;float layerA=a*wave(phaseA),layerB=b*wave(phaseB);phaseA+=hz*(1.f+mod*lfo*.02f)/sr;phaseB+=hz*std::pow(2.f,(semi+det/100.f)/12.f)*(1.f+mod*lfo*.02f)/sr;phaseA-=std::floor(phaseA);phaseB-=std::floor(phaseB);if(sample&&sample->getNumSamples()>1){int ix=(int)samplePos;if(ix<sample->getNumSamples()-1){float frac=samplePos-ix;float sm=sample->getSample(0,ix)*(1-frac)+sample->getSample(0,ix+1)*frac;layerA=sm*a;layerB=sm*b;}samplePos+=(double)juce::MidiMessage::getMidiNoteInHertz(note)/juce::MidiMessage::getMidiNoteInHertz(60)*owner.importedSampleRate/sr;}auto stage=[&](juce::dsp::StateVariableTPTFilter<float>& f,int t,float input){float bp=f.processSample(0,input);if(t==3)return input-.8f*bp;if(t==4)return input+.8f*bp;return bp;};float x=stage(f2,(int)owner.param("f2type"),stage(f1,(int)owner.param("f1type"),layerA+layerB));float amp=env.getNextSample()*velocity*.24f*owner.param("master");float dryGain=amp;x*=amp;if(out.getNumChannels()>0)out.addSample(0,start+i,x*gl);if(out.getNumChannels()>1)out.addSample(1,start+i,x*gr);if(dryA.getNumChannels()>0)dryA.addSample(0,start+i,layerA*dryGain);if(dryB.getNumChannels()>0)dryB.addSample(0,start+i,layerB*dryGain);}
  if(!env.isActive())clearCurrentNote();
 }
private:
 GemS2Processor& owner; juce::ADSR env; juce::dsp::StateVariableTPTFilter<float> f1,f2; std::shared_ptr<const juce::AudioBuffer<float>> sample; int note=60;float velocity=1,phaseA=0,phaseB=0,lfoPhase=0,pitchBend=0,modWheel=0;double samplePos=0;
 float wave(float p){return std::sin(juce::MathConstants<float>::twoPi*p);}
};

GemS2Processor::GemS2Processor()
 : AudioProcessor(BusesProperties().withOutput("Wet Stereo",juce::AudioChannelSet::stereo(),true).withOutput("Dry Out 1",juce::AudioChannelSet::mono(),false).withOutput("Dry Out 2",juce::AudioChannelSet::mono(),false).withOutput("Dry Out 3",juce::AudioChannelSet::mono(),false).withOutput("Dry Out 4",juce::AudioChannelSet::mono(),false)),
   state(*this,nullptr,"GEM_S2_STATE",makeParameters()) {formats.registerBasicFormats();for(int i=0;i<32;++i)synth.addVoice(new Voice(*this));synth.addSound(new SynthSound());}
float GemS2Processor::param(const char* id) const { return const_cast<GemS2Processor*>(this)->state.getRawParameterValue(id)->load(); }
void GemS2Processor::setPitchWheel(int value){for(int ch=1;ch<=16;++ch)synth.handlePitchWheel(ch,juce::jlimit(0,16383,value));}
void GemS2Processor::setModWheel(int value){for(int ch=1;ch<=16;++ch)synth.handleController(ch,1,juce::jlimit(0,127,value));}

juce::AudioProcessorValueTreeState::ParameterLayout GemS2Processor::makeParameters(){
 using F=juce::AudioParameterFloat;
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto add=[&](const char* id,const char* name,float lo,float hi,float v){p.add(std::make_unique<F>(id,name,juce::NormalisableRange<float>(lo,hi),v));};
 add("osc1level","Layer A Level",0,1,.8f); add("osc2level","Layer B Level",0,1,.5f);
 add("osc2semi","Layer B Transpose",-24,24,0); add("detune","Detune",-100,100,0);
 add("f1cut","Filter 1 Cutoff",20,20000,12000); add("f1res","Filter 1 Resonance",0,1,.15f);
 add("f2cut","Filter 2 Cutoff",20,20000,10000); add("f2res","Filter 2 Resonance",0,1,.1f);
 p.add(std::make_unique<juce::AudioParameterChoice>("f1type","Filter 1 Type",juce::StringArray{"Low Pass","High Pass","Band Pass","Parametric Cut","Parametric Boost"},0));
 p.add(std::make_unique<juce::AudioParameterChoice>("f2type","Filter 2 Type",juce::StringArray{"Low Pass","High Pass","Band Pass","Parametric Cut","Parametric Boost"},0));
 p.add(std::make_unique<juce::AudioParameterChoice>("dryA","Layer A Dry Output",juce::StringArray{"Out 1","Out 2","Out 3","Out 4"},0));p.add(std::make_unique<juce::AudioParameterChoice>("dryB","Layer B Dry Output",juce::StringArray{"Out 1","Out 2","Out 3","Out 4"},1));
 add("attack","Attack",.001f,10,.02f); add("decay","Decay",.001f,10,.4f);
 add("sustain","Sustain",0,1,.75f); add("release","Release",.001f,20,1.2f);
 add("lforate","LFO Rate",.01f,20,1); add("lfodepth","LFO Depth",0,1,0);
 add("reverb","Reverb Mix",0,1,.2f); add("chorus","Chorus Mix",0,1,.1f);
 add("pan","Pan",-1,1,0); add("master","Master Level",0,1,.8f);
 return p;
}
void GemS2Processor::prepareToPlay(double sr,int bs){synth.setCurrentPlaybackSampleRate(sr);for(int i=0;i<synth.getNumVoices();++i)if(auto* v=dynamic_cast<Voice*>(synth.getVoice(i)))v->prepare(sr);scratch.setSize(2,bs);juce::dsp::ProcessSpec spec{sr,(juce::uint32)bs,2};chorus.prepare(spec);chorus.reset();juce::dsp::Reverb::Parameters rp;rp.roomSize=.55f;rp.damping=.45f;rp.wetLevel=param("reverb");rp.dryLevel=1.f-rp.wetLevel;reverb.setParameters(rp);reverb.reset();}
void GemS2Processor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals no;keyboardState.processNextMidiBuffer(midi,0,b.getNumSamples(),true);b.clear();synth.renderNextBlock(b,midi,0,b.getNumSamples());juce::Reverb::Parameters rp;rp.roomSize=.55f;rp.damping=.45f;rp.wetLevel=param("reverb");rp.dryLevel=1.f-rp.wetLevel;reverb.setParameters(rp);auto wet=getBusBuffer(b,false,0);if(wet.getNumChannels()>1){juce::dsp::AudioBlock<float> block(wet);juce::dsp::ProcessContextReplacing<float> context(block);chorus.setRate(.25f+param("lforate")*.2f);chorus.setDepth(.35f);chorus.setMix(param("chorus"));chorus.process(context);reverb.processStereo(wet.getWritePointer(0),wet.getWritePointer(1),wet.getNumSamples());}else if(wet.getNumChannels()==1)reverb.processMono(wet.getWritePointer(0),wet.getNumSamples());
}
void GemS2Processor::loadSampleFile(const juce::File& file){std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));if(!reader){importStatus="Unsupported sample format";return;}auto data=std::make_shared<juce::AudioBuffer<float>>((int)reader->numChannels,(int)reader->lengthInSamples);reader->read(data.get(),0,(int)reader->lengthInSamples,0,true,false);importedSampleRate=reader->sampleRate;std::atomic_store(&sampleData,std::shared_ptr<const juce::AudioBuffer<float>>(data));importStatus="Loaded: "+file.getFileName();}
bool GemS2Processor::importSysEx(const juce::File& file){juce::FileInputStream in(file);if(!in.openedOk()){importStatus="Cannot open SysEx";return false;}juce::MemoryBlock bytes;in.readIntoMemoryBlock(bytes);auto* d=(const uint8_t*)bytes.getData();size_t n=bytes.getSize(),pos=0,found=0;juce::MemoryBlock extracted;while(pos<n){while(pos<n&&d[pos]!=0xF0)++pos;if(pos>=n)break;size_t end=pos+1;while(end<n&&d[end]!=0xF7)++end;if(end>=n)break;size_t len=end-pos+1;if(len>=8&&d[pos+1]==0x2F&&d[pos+2]==0x5C&&d[pos+3]==0x0D){uint8_t check=0;for(size_t i=pos+1;i<end;++i)check^=d[i];if(check==0){extracted.append(d+pos,len);++found;}}pos=end+1;}if(!found){importStatus="No valid GEM DATA_DUMP packets (F0 2F 5C 0D)";return false;}auto out=file.getSiblingFile(file.getFileNameWithoutExtension()+"_payload.gemdump");if(!out.replaceWithData(extracted.getData(),extracted.getSize())){importStatus="Packets valid, unable to write payload";return false;}importStatus=juce::String((int)found)+" valid DATA_DUMP packets extracted; patch decoding still requires matching Turbo field map";return true;}
juce::String GemS2Processor::inspectDiskImage(const juce::File& file){
 juce::FileInputStream in(file);if(!in.openedOk())return "Cannot open disk image";juce::MemoryBlock m;in.readIntoMemoryBlock(m);const auto* d=(const uint8_t*)m.getData();const int n=(int)m.getSize();if(n<512||m.getSize()!=1474560)return "Expected a 1.44 MB FAT12 floppy image";
 auto u16=[&](int p){return (int)d[p]|((int)d[p+1]<<8);};const int bps=u16(11),spc=d[13],reserved=u16(14),fats=d[16],roots=u16(17),spf=u16(22);if(bps!=512||spc<1||fats<1)return "Unsupported FAT12 geometry";const int rootS=(roots*32+bps-1)/bps,dataStart=reserved+fats*spf+rootS,fatOff=reserved*bps;if(dataStart*bps>=n)return "Invalid FAT12 offsets";
 auto next=[&](int c){int o=(c*3)/2;if(fatOff+o+1>=n)return 0xFF7;int v=u16(fatOff+o);return (c&1)?(v>>4):(v&0xFFF);};
 auto readChain=[&](int c,int size){juce::MemoryBlock out;for(int guard=0;guard<4096&&c>=2&&c<0xFF0&&(int)out.getSize()<size;++guard){int off=(dataStart+(c-2)*spc)*bps;if(off<0||off+spc*bps>n)break;out.append(d+off,(size_t)spc*bps);int nc=next(c);if(nc>=0xFF8)break;c=nc;}if(size>=0&&(int)out.getSize()>size)out.setSize((size_t)size);return out;};
 int samples=0,sounds=0,perfs=0,songs=0,banks=0;std::vector<juce::String> soundNames;std::function<void(int,const juce::String&,bool)> walk;
 walk=[&](int cluster,const juce::String& path,bool root){juce::MemoryBlock dir;if(root){int off=(reserved+fats*spf)*bps;int sz=roots*32;if(off+sz>n)return;dir.append(d+off,(size_t)sz);}else dir=readChain(cluster,-1);auto* q=(const uint8_t*)dir.getData();for(int i=0;i+31<(int)dir.getSize();i+=32){if(q[i]==0)break;if(q[i]==0xE5||q[i+11]==15)continue;int attr=q[i+11];char base[9]{},ext[4]{};std::memcpy(base,q+i,8);std::memcpy(ext,q+i+8,3);juce::String bn=juce::String::fromUTF8(base).trimEnd(),ex=juce::String::fromUTF8(ext).trimEnd();if(bn=="."||bn=="..")continue;juce::String name=ex.isNotEmpty()?bn+"."+ex:bn;int cl=(int)q[i+26]|((int)q[i+27]<<8);int size=(int)q[i+28]|((int)q[i+29]<<8)|((int)q[i+30]<<16)|((int)q[i+31]<<24);juce::String p=path.isEmpty()?name:path+"/"+name;if(attr&16){if(path.isEmpty()&&bn.startsWith("BANK"))++banks;walk(cl,p,false);}else{if(name.endsWithIgnoreCase(".SAM"))++samples;else if(path.containsIgnoreCase("/SOUNDS"))++sounds;else if(path.containsIgnoreCase("/PERF"))++perfs;else if(name.startsWithIgnoreCase("Song"))++songs;if(name=="SOUNDMAP"){auto data=readChain(cl,size);auto* s=(const uint8_t*)data.getData();for(int z=6;z+17<(int)data.getSize();z+=18){char nm[11]{};std::memcpy(nm,s+z,10);juce::String sn=juce::String::fromUTF8(nm).trimEnd();if(sn.isNotEmpty()&&soundNames.size()<12)soundNames.push_back(sn+" ["+juce::String((int)s[z+12])+"-"+juce::String((int)s[z+13])+"]");}}}}};
 walk(0,{},true);if(samples==0&&sounds==0)return "FAT12 parsed, but no S-series content found";juce::String report="FAT12 OK | "+juce::String(banks)+" banks, "+juce::String(perfs)+" performances, "+juce::String(songs)+" songs, "+juce::String(sounds)+" sound files, "+juce::String(samples)+" samples";for(auto& sn:soundNames)report+="\n"+sn;importStatus=report;return report;
}
void GemS2Processor::getStateInformation(juce::MemoryBlock& d){auto x=state.copyState();std::unique_ptr<juce::XmlElement> xml(x.createXml());copyXmlToBinary(*xml,d);}
void GemS2Processor::setStateInformation(const void* d,int n){std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d,n));if(x&&x->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new GemS2Processor();}

