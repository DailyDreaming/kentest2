    $(document).ready(function(){

            $("a.change").each(function(){ 
                    this.href = this.href.replace(/^http:\/\/genome\.ucsc\.edu/, "http://" +window.location.host);
                });
            
            $("a.insideLink").each(function(){ 
                    this.href = this.href.replace(/=http:\/\/genome\.ucsc\.edu/, "=http://" +window.location.host);
                });
            $("a.changeHg").each(function(){ 
                    this.href = this.href.replace(/^http:\/\/hgdownload\.cse\.ucsc\.edu/, "http://" +window.location.host);
                });
            $("a.changeHgFtp").each(function(){ 
                    this.href = this.href.replace(/^ftp:\/\/hgdownload\.cse\.ucsc\.edu/, "ftp://" +window.location.host);
                });

            if (location.host == 'genome-euro.ucsc.edu' || location.host == 'genome-asia.ucsc.edu'){
            $("a.euro").each(function(){
                    this.href = this.href.replace(/^http:\/\/genome\.ucsc\.edu/, "http://" +window.location.host);
                });
            }


     });
     
