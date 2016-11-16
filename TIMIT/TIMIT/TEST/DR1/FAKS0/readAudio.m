function out = readAudio(fname,rangeSec,bRand)
%readAudio   Read a wavefile to an audio object.
%
%USAGE
%   aObj = readAudio(fname);
%   aObj = readAudio(fname,rangeSec,bRand);
%
%INPUT ARGUMENTS
%      fname : string/cell specifying wavefile which should be be loaded.
%   rangeSec : limit audio data length in seconds [lenthSec]
%                 or audio data range  in seconds [startSec endSec]
%              (default, rangeSec = [0 inf])
%      bRAND : a random offset is added to the chosen rangeSec
%              (default, bRAND = false)
% 
%OUTPUT ARGUMENTS
%   aObj : audio object
%
%NOTE
%   The specified audio range in seconds "rangeSec" is limited to be within
%   the sample range [1 maxSamples] of the wave file "fname".
% 
%   See also genAudio, isAudio, normalizeAudio, updateAudio and plotAudio.

%   Developed with Matlab 7.4.0.287 (R2007a). Please send bug reports to:
% 
%   Author  :  Tobias May, ? 2007-2009
%              TUe Eindhoven and Philips Research  
%              t.may@tue.nl      tobias.may@philips.com
%
%   History : 
%   v.0.1   2007/08/17
%   v.0.2   2009/10/20 cleaned up
%   v.0.3   2009/11/23 added random offset
%   v.0.4   2009/11/26 added TIMIT support
%   ***********************************************************************


%% ***********************  CHECK INPUT ARGUMENTS  ************************

% Check for proper input arguments
checkInArg(1,3,nargin,mfilename);

% Set default values
if nargin < 3 || isempty(bRand);    bRand  = false; end
if nargin < 2 || isempty(rangeSec); bLimit = false;
else
    bLimit = true;
    % Check if "rangeSec" is an integer
    if numel(rangeSec) == 1
        rangeSec = [0 rangeSec];
    elseif numel(rangeSec) > 2
        error(['"rangeSec" must be either a scalar specifying the '   ,...
               'audio length in seconds [lenthSec], or a vector with ',...
               'two elements specifying the audio range in seconds '  ,...
               '[startSec endSec].']);
    end
end

% Check fname
if ischar(fname)
   % Do nothing...
elseif iscell(fname)
    % Convert cell to string
    fname = fname{:};
else
    % FNAME is neither string nor cell ...
    error(['"fname" must be either a string or a cell specifying the',...
           ' name of a WAVE FILE which should be loaded.']);
end

% Check for proper file extension
[tmp,fileName,fileExt] = fileparts(fname); %#ok

% Add WAVE file extension
if isempty(fileExt)
    fname = strcat(fileName,'.wav');
end

% Check if file "fname" exists
if ~exist(fname,'file')
    error(['Cannot open WAVE file "',fname,'".']);
else
    % Try to figure out if TIMIT database is used ...
    if exist([fname(1:end-3) 'phn'],'file') || ...
       exist([fname(1:end-3) 'PHN'],'file');
        % Found phonetic transcription file
        format = 'TIMIT';
    else
        format = 'WAV';
    end
end


%% ***************************  READ WAVE FILE  ***************************

% Select format
switch upper(format)
    case 'TIMIT'
        % 16 bit PCM @ 16kHz, mono
        fs = 16e3; nBits = 16; nChannels = 1;
        % Read TIMIT audio file
        [x,phoneme,endpoints] = wavReadTimit(fname);
        % Scale data to be within [-1; 1].
        x = x / 2^(nBits-1);
        % Determine silence, unvoiced, and voiced segments.
        timeSeg = TimitSegMarks(phoneme,endpoints);
        % Determine total number of samples
        nSamples = min(size(timeSeg,1),size(x,2));
        
        % Limit audio file length
        if bLimit
            range(1) = max(round(rangeSec(1)*fs),1);
            range(2) = min(size(timeSeg,1),min(rangeSec(2)*fs,size(x,2)));
            % Randomize start point of audio file
            if bRand
                % Get random offset
                offset = rnsubset(1,dim(1) - range(2));
                % Add offset to data range
                range = range + offset;
            end
            % Convert x to column vector
            data = double(x(range(1):range(2)).');
            nSamples = diff(range)+1;
        else
            % Convert x to column vector
            data = double(x(1:nSamples)).';
            % Determine size
            [nSamples, nChannels] = size(data);
            range = [1 nSamples];
        end        
        
        
    case 'WAV'
        % Read wave file
        [dim,fs,nBits] = wavread(fname,'size');

        % Limit audio length
        if bLimit
            range(1) = max(round(rangeSec(1)*fs),1);
            range(2) = min(round(rangeSec(2)*fs),dim(1));
            
            % Randomize start point of audio file
            if bRand
                % Get random offset
                offset = rnsubset(1,dim(1) - range(2));
                % Add offset to data range
                range = range + offset;
            end
            
            nSamples = diff(range)+1;
        else
            % Read complete file
            range    = [1 dim(1)];
            nSamples = dim(1);
        end
        
        % Number of channels
        nChannels = dim(2);
        
        % Read audio data
        data = wavread(fname,range);
    otherwise
        error(['Format "',upper(format),'" is not recognized.'])
end

% TODO % Add support of other audio formats


%% *************************  CREATE AUDIO OBJECT  ************************

% Create audio object
out = struct('label',fname,'data',data,'fs',fs,'nBits',nBits,...
             'nSamples',nSamples,'nChannels',nChannels,'range',range,...
             'format',format);


%% **************************  PLOT AUDIO OBJECT  *************************

% % Plot signal
% if nargout == 0
%     plotAudio(out);
% end


%   ***********************************************************************
%   This program is free software: you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation, either version 3 of the License, or
%   (at your option) any later version.
% 
%   This program is distributed in the hope that it will be useful,
%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%   GNU General Public License for more details.
% 
%   You should have received a copy of the GNU General Public License
%   along with this program.  If not, see <http://www.gnu.org/licenses/>.
%   ***********************************************************************