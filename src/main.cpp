#include <dpp/cache.h>
#include <dpp/discordvoiceclient.h>
#include <dpp/dispatcher.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>
#include <dpp/dpp.h>
#include <dpp/intents.h>
#include <dpp/utility.h>
#include <sstream>
#include <iomanip>

#include <string.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <lame/lame.h>
#include <mpg123.h>
#include <out123.h>
#include <sys/stat.h>

const std::string    BOT_TOKEN    = "lol asdlol";

void toMP3(std::string name) {
	try {
		int read, write;
		FILE *pcm = fopen(std::string(("./"+name+".pcm")).c_str(),"rb");
		//fseek(pcm,4*1024,SEEK_CUR);

		FILE *mp3 = fopen(std::string(("./"+name+".mp3")).c_str(),"wb");

		const int PCM_SIZE = 8192*3;
		const int MP3_SIZE = 8192*3;
		short int pcm_buffer[PCM_SIZE*2];
		unsigned char mp3_buffer[MP3_SIZE];

		lame_t lame = lame_init();
		lame_set_in_samplerate(lame, 48000);
		lame_set_VBR(lame, vbr_default);
		lame_init_params(lame);

		int nTotalRead = 0;
		do {
			read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);

			nTotalRead+=read*4;

			if(read == 0) {
				write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
			} else {
				write = lame_encode_buffer_interleaved(lame,pcm_buffer,read,mp3_buffer, MP3_SIZE);
			}

			fwrite(mp3_buffer,write,1,mp3);

		} while ( read!=0 );

		lame_close(lame);
		fclose(mp3);
		fclose(pcm);
	} catch (int e) {
		std::string error = "Lol something went wrong saving the audio data as an mp3 with error " + std::to_string(e) + " gl figuring it out bc idk hpow";
		std::cout << error << std::endl;
	}
}

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat (name.c_str(),  &buffer) == 0);
}

std::vector<uint8_t> video(std::string url, const dpp::message_create_t& event) {

	system(std::string("yt-dlp -x -k -f bestaudio/best --audio-format mp3 \"" + url + "\" -o  \"./youtube/latest.mp3\"" ).c_str());
	event.reply("Playing song");
    std::vector<uint8_t> pcmdata;

    mpg123_init();

    int err = 0;
    unsigned char* buffer;
    size_t buffer_size, done;
    int channels, encoding;
    long rate;

    mpg123_handle *mh = mpg123_new(NULL, &err);
    mpg123_param(mh, MPG123_FORCE_RATE, 48000, 48000.0);

    buffer_size = mpg123_outblock(mh);
    buffer = new unsigned char[buffer_size];

    mpg123_open(mh, "/home/marroq/Spy/youtube/latest.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    unsigned int counter = 0;
    for (int totalBytes = 0; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK; ) {
        for (auto i = 0; i < buffer_size; i++) {
            pcmdata.push_back(buffer[i]);
        }
        counter += buffer_size;
        totalBytes += done;
    }
    delete buffer;
    mpg123_close(mh);
    mpg123_delete(mh);

	return pcmdata;
}

int main() {


    dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);

	const dpp::snowflake brandon_id = 235192366368030720;
	const dpp::snowflake dom_id = 309476658023366657;
	const dpp::snowflake dylan_id = 220338791481606144;
	const dpp::snowflake zack_id = 235216438942629888;

	FILE* brandon;
	FILE* dom;
	FILE* dylan;
	FILE* zack;

    bot.on_log(dpp::utility::cout_logger());

	bot.on_message_create([&bot, &brandon, &dom, &dylan, &zack](const dpp::message_create_t& event) {
		std::string msg = event.msg.content;
		std::stringstream ss(event.msg.content);
		std::string command;
		ss >> command;

		if (command == ".record") {
			brandon = fopen("./recordings/brandon.pcm", "wb");
			dom = fopen("./recordings/dom.pcm", "wb");
			dylan = fopen("./recordings/dylan.pcm", "wb");
			zack = fopen("./recordings/zack.pcm", "wb");
		}

		if(command == ".leave") {
			event.from->disconnect_voice(event.msg.guild_id);
		}

		if(command == ".stop") {
			dpp::voiceconn* v = event.from->get_voice(event.msg.guild_id);
			if(v->voiceclient->is_playing()) {
				v->voiceclient->stop_audio();
			}
		}

		if(command == ".disconnect") {
			event.from->disconnect_voice(event.msg.guild_id);
		}

		if(command == ".save_rec") {
			toMP3("brandon");
			toMP3("dom");
			toMP3("dylan");
			toMP3("zack");
			fclose(brandon);
			fclose(dom);
			fclose(dylan);
			fclose(zack);
			dpp::message msg (event.msg.channel_id, "Saving");
			msg.add_file("brandon.mp3", dpp::utility::read_file("./recordings/brandon.mp3"));
			msg.add_file("dom.mp3", dpp::utility::read_file("./recordings/dom.mp3"));
			msg.add_file("dylan.mp3", dpp::utility::read_file("./recordings/dylan.mp3"));
			msg.add_file("zack.mp3", dpp::utility::read_file("./recordings/zack.mp3"));

			event.reply(msg);
		}

		if(command == ".join") {
			dpp::guild* g = dpp::find_guild(event.msg.guild_id);
            if (!g->connect_member_voice(event.msg.author.id)) {
				event.reply("You need to be in a voice channel");
            }
		}

		if (command == ".play") {
			if(fileExists(std::string("./youtube/latest.webm"))) {
				system("rm ./youtube/latest.webm");
			}
			if(fileExists(std::string("./youtube/latest.mp3"))) {
				system("rm ./youtube/latest.mp3");
			}
			std::string url = msg.substr(6,msg.length());
			if(!event.from->get_voice(event.msg.guild_id)) {
				event.reply("Bot is not in VC, do .join");
			} else {
				dpp::voiceconn* v = event.from->get_voice(event.msg.guild_id);
				if(v && v->voiceclient && v->voiceclient->is_playing()) {
					v->voiceclient->stop_audio();
				} else if (v && v->voiceclient && v->voiceclient->is_ready()) {
					event.reply("Starting download...");
					std::vector<uint8_t> pcmdata = video(url, event);
					v->voiceclient->send_audio_raw((uint16_t*)pcmdata.data(), pcmdata.size());

				}
			}

		}

	});

	bot.on_voice_receive([&bot,&brandon,&dom,&dylan,&zack,&brandon_id,&dom_id,&dylan_id,&zack_id](const dpp::voice_receive_t& event) {
		if(event.user_id == brandon_id) {
			fwrite((char *)event.audio, 1, event.audio_size, brandon);
		}
		if(event.user_id == dom_id) {
			fwrite((char *)event.audio, 1, event.audio_size, dom);
		}
		if(event.user_id == dylan_id) {
			fwrite((char *)event.audio, 1, event.audio_size, dylan);
		}
		if(event.user_id == zack_id) {
			fwrite((char *)event.audio, 1, event.audio_size, zack);
		}
	});

    bot.start(false);
	mpg123_exit();
	return 0;
}
