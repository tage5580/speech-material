function [classes]=TimitSegMarks(phonemes,endmarks)
%
% [mp,sm]=TimitSegMarks(phonemes)
%
% Classifies the TIMIT phoneme classes to silence, unvoiced, and voiced
% segments. 
%
% A. Harma, Philips Research, 04-07-2005

voiced=char('iy','ih','eh','ey','ae','aa','aw','ay','ah','ao',...% Vowels
                    'oy','ow','ox','er','ax','ix','axr','ax-h',...
                    'm','n','ng','em','en','eng','nx',... % Nasals
                    'l','r','w','el'); % Semivowels                     
sil=char('pau','epi','h#');            % Silences        
Nv=size(voiced,1);
Ns=size(sil,1);
N=size(phonemes,1);
classes=zeros(max(endmarks(:,2)),1);
for q=1:N,
    vuv=0; % Unvoiced 
    % Voiced
    for w=1:Nv,
        if ~isempty(findstr(deblank(phonemes(q,:)), deblank(voiced(w,:)))),
           vuv=1;                
        end
    end
    if vuv~=1,
        % Silence ?
        for w=1:Ns,
            if ~isempty(findstr(deblank(phonemes(q,:)), deblank(sil(w,:)))),
                vuv=-1; 
            end
        end
    end
    classes(endmarks(q,1):endmarks(q,2))=vuv*ones(endmarks(q,2)-endmarks(q,1)+1,1); 
end
   
            
        
        