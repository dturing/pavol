#include <stdio.h>
#include <stdlib.h>
#include <popt.h>
#include <pulse/pulseaudio.h>
#include <string.h>

#define ASSERT(a,b) if(!(a)) { fprintf(stderr,"Failed to " b "\n"); exit(-1); }
#define LOG(format,...) fprintf( stderr, format, ##__VA_ARGS__)

int mute = 0;
int vol = -1;
struct pa_cvolume volume;
pa_mainloop *mainloop;
int operations = 0;

void usage( poptContext optCon, int exitcode, char *error, char *addl ) {
	poptPrintUsage(optCon, stderr, 0);
	if( error ) fprintf( stderr, "%s: %s\n", error, addl );
	exit( exitcode );
}

void success_cb( pa_context *ctxt, int success, void *userdata ) {
	LOG("Operation success: %i\n", success );
	operations--;
	if( operations == 0 ) pa_mainloop_quit( mainloop, 0 );
}

void dump_sink_info( pa_context *ctxt, const pa_sink_info *i, int eol, void* userdata ) {
	if( i ) {
		LOG("Got src info: [%i] %s, %s\n", i->index, i->name, i->description );
		
		ASSERT( pa_context_set_sink_mute_by_index( ctxt, i->index, mute, success_cb, NULL ), "mute" );
		if( vol!=-1 )
			pa_context_set_sink_volume_by_index( ctxt, i->index, &volume, success_cb, NULL );
		operations++;
	}
}

void context_state_cb( pa_context *ctxt, void *userdata ) {
	ASSERT( ctxt, "initialize context" );
	
	switch( pa_context_get_state(ctxt) ) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY: {
			ASSERT( pa_context_get_sink_info_list( ctxt, dump_sink_info, NULL ), "query outputs (sinks)" );

			break;
		}
		case PA_CONTEXT_TERMINATED:
			LOG("context terminated\n");
			break;
		case PA_CONTEXT_FAILED:
		default:
			LOG("initialize PA connection: %s\n", pa_strerror(pa_context_errno(ctxt)));
			exit(-2);
	}
}


int main( int argc, const char *argv[] ) {
	const char *pa_server = NULL;
	char c;
	int quiet = 0;

	poptContext optCon;
	struct poptOption optionsTable[] = {
		{ "server", 'h', POPT_ARG_STRING, &pa_server, 0, "PulseAudio server to connect to", "" },
		{ "mute", 'm', 0, 0, 'm', "mute (else, unmute)", "" },
		{ "volume", 'v', POPT_ARG_INT, &vol, 0, "Volume (0-100)", "" },
		{ "quiet", 'q', 0, 0, 'q', "be quiet", "" },
		POPT_AUTOHELP
		{ NULL, 0, 0, NULL, 0 }
	};
	optCon = poptGetContext( NULL, argc, argv, optionsTable, 0 );
	
	while( (c=poptGetNextOpt(optCon)) >= 0 && c!=255) {
		switch( c ) {
			case 'q':
				quiet++;
				break;
			case 'm':
				mute=1;
				break;
			default:
				fprintf( stderr, "unknown option '%c' (%i)\n",c,c );
				break;
		}
	}
	
	if( c < -1 ) {
		fprintf( stderr, "%s: %s\n", poptBadOption( optCon, POPT_BADOPTION_NOALIAS),
			poptStrerror(c) );
		return 1;
	}
	
	int r;

	mainloop = pa_mainloop_new();
	pa_context *ctxt = pa_context_new( pa_mainloop_get_api(mainloop), NULL );
	ASSERT( ctxt!=NULL, "initialize PA context" );

	r = pa_context_connect( ctxt, pa_server, (pa_context_flags_t)0, NULL );
	ASSERT( r==0, "connect to PA context" );

	pa_context_set_state_callback( ctxt, context_state_cb, NULL );
	
	volume.channels = 2;
	volume.values[0] = volume.values[1] = (vol*PA_VOLUME_NORM)/100;

	pa_mainloop_run( mainloop, &r );
	
	pa_context_unref( ctxt );
	pa_mainloop_free( mainloop );

	return r;
}

