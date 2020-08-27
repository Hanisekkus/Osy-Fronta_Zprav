#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <mqueue.h>
#include <string.h>


int FORK_NUMBER = 2;


ssize_t readLine( int fd, void *buf, size_t count )
{

    static char localStorage[ 2048 ];
    static char overLoadStorage[ 2048 ];
    
    static int offset = 0;
    static int lenght = 0;
    static int overLoad = 0;

    int counterNewLine = 0;


    if( lenght - offset == 0 || overLoad )
    {

        offset = 0;
        lenght = read( fd, localStorage, 2048 );

    }
    
    for( int i = offset; i <= lenght; i ++ )
    {

        if( localStorage[ i ] == '\n' )
        {

            counterNewLine = i - offset + 1;
            break;

        }

        if( i == lenght )
        {

            memcpy( overLoadStorage, localStorage + offset, lenght - offset );
            overLoad = lenght - offset;

            return readLine( fd, buf, count );

        }

    }

    if( overLoad )
    {

        strncat( overLoadStorage, localStorage, overLoad );
        memcpy( buf, overLoadStorage, counterNewLine + overLoad );

        offset += counterNewLine;  
        counterNewLine += overLoad;
        overLoad = 0;
        return counterNewLine;

    }else
    {

        memcpy( buf, localStorage + offset, counterNewLine );
    
    }

    if( counterNewLine > count )
    {

        return -1;

    }

    offset += counterNewLine;  

    return counterNewLine;

}


int main( int num, char **args )
{

    setbuf( stdout, NULL );

    if( ! ( num <= 1 ) )
    {

        FORK_NUMBER = atoi( args[ 1 ] );

    }

    struct mq_attr mqatr = { 0, 10, sizeof( int ), 0};
    int cislo = 0;

    mq_unlink( "/vyroba" );
    mqd_t mq_vyroba = mq_open( "/vyroba", O_RDWR | O_CREAT, 0600, &mqatr );
    mq_send( mq_vyroba, ( char * ) &cislo, sizeof( cislo ), 0 );

    mq_unlink( "/spotreba" );
    mqd_t mq_spotreba = mq_open( "/spotreba", O_RDWR | O_CREAT, 0600, &mqatr );
    
    int count = 0;
    int multiplier = 0;
    int pipeSys[ 2 ];
     
    pipe( pipeSys );

    for( int i = 0; i < FORK_NUMBER; i++ )
    {// -- Vyroba potomku

        if( fork() == 0)
        {// -- Potomek

            while( 1 )
            {

                mq_receive( mq_vyroba, ( char * ) &cislo, sizeof( cislo ), 0 );
                // -- Logika ve fronte zprav pro potomka

                dprintf( pipeSys[ 1 ], "Potomek [%d]\n", getpid() );
                mq_send( mq_spotreba, ( char * ) &cislo, sizeof( cislo ),0 );

            }

        }

    }
    while( 1 )
    {// -- Rodic
     
        mq_receive( mq_spotreba, ( char * ) &cislo, sizeof( cislo ), 0 );
        // -- Logika ve fronte zprav pro rodice 

        char buf[ 256 ];
        int lenght = readLine( pipeSys[ 0 ], buf, sizeof( buf ) );
        
        if( lenght <= 0 )
            dprintf( 1, "Problem na strane readLine\n" );

        count ++;

        dprintf( 1, "[row:%d][multiplier:%d] Rodic [%d] cte:", count, multiplier, getpid() );
        write( 1, buf, lenght );
        dprintf( 1, "\n" );

        if( count % 10 == 0 )
        {

            count = 0;
            multiplier ++;
            usleep( 100000 );

        }
        mq_send( mq_vyroba, ( char * ) &cislo, sizeof( cislo ),0 );

    }

    return 0;

}