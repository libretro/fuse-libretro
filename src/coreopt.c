#include <coreopt.h>
#include <string.h>

int coreopt( retro_environment_t env_cb, const struct retro_variable* vars, const char* opt_name, const char** opt_value )
{
  const struct retro_variable* var = vars;
  
  while ( var->key )
  {
    if ( !strcmp( var->key, opt_name ) )
    {
      break;
    }
    
    ++var;
  }
  
  if ( var->key )
  {
    const char* first_opt = strchr( var->value, ';' );

    if ( first_opt )
    {
      while ( *++first_opt == ' ' ) /* do nothing */;

      struct retro_variable user_opt;

      user_opt.key = var->key;
      user_opt.value = NULL;

      if ( env_cb( RETRO_ENVIRONMENT_GET_VARIABLE, &user_opt ) && user_opt.value )
      {
        const char* option = first_opt;
        size_t opt_len = strlen( user_opt.value );

        for ( ;; )
        {
          option = strstr( option, user_opt.value );

          if ( !option )
          {
            break;
          }

          if ( ( option == first_opt || option[ -1 ] == '|' ) || ( option[ opt_len ] == 0 || option[ opt_len ] == '|' ) )
          {
            int pipes = 0;

            while ( option > first_opt )
            {
              pipes += *--option == '|';
            }

            if ( opt_value )
            {
              *opt_value = user_opt.value;
            }
            
            return pipes;
          }

          option += opt_len;
        }
      }
    }
  }
  
  return -1;
}
