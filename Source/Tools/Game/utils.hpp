#pragma once

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief Convert String to Number
 */
template <typename TP>
TP str2num( std::string const& value ){

    std::stringstream sin;
    sin << value;
    TP output;
    sin >> output;
    return output;
}


/**
 * @brief Convert number to string
 */
template <typename TP>
std::string num2str( TP const& value ){
    std::stringstream sin;
    sin << value;
    return sin.str();
}


/**
 * @brief Execute Generic Shell Command
 *
 * @param[in]   command Command to execute.
 * @param[out]  output  Shell output.
 * @param[in]   mode read/write access
 *
 * @return 0 for success, 1 otherwise.
 *
*/
int Execute_Command( const std::string&  command,
                     std::string&        output,
                     const std::string&  mode = "r")
{
    // Create the stringstream
    std::stringstream sout;

    // Run Popen
    FILE *in;
    char buff[512];

    // Test output
    if(!(in = popen(command.c_str(), mode.c_str()))){
        return 1;
    }

    // Parse output
    while(fgets(buff, sizeof(buff), in)!=NULL){
        sout << buff;
    }

    // Close
    int exit_code = pclose(in);

    // set output
    output = sout.str();

    // Return exit code
    return exit_code;
}


/**
 * @brief Ping
 *
 * @param[in] address Address to ping.
 * @param[in] max_attempts Number of attempts to try and ping.
 * @param[out] details Details of failure if one occurs.
 *
 * @return True if responsive, false otherwise.
 *
 * @note { I am redirecting stderr to stdout.  I would recommend
 *         capturing this information separately.}
 */
bool Ping( const std::string& address,
           const int&         max_attempts,
           std::string&       details )
{
    // Format a command string
    std::string command = "ping -c " + num2str(max_attempts) + " -W1 " + address + " 2>&1";
    std::string output;

    // Execute the ping command
    int code = Execute_Command( command, details );

    return (code == 0);
}
