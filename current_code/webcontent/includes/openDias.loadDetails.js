// Read a page's GET URL variables and return them as an associative array.
function getUrlVars() {
    var vars = [], hash;
    var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
    for(var i = 0; i < hashes.length; i++) {
        hash = hashes[i].split('=');
        vars.push(hash[0]);
        vars[hash[0]] = hash[1];
    }
    return vars;
}

$(document).ready(function() {

  $.ajax({ url: "/opendias/dynamic",
         dataType: "xml",
         timeout: 10000,
         data: {action: "getDocDetail", 
                docid: getUrlVars()['docid']},
         cache: false,
         type: "POST",
         error: function( x, t, m ) {
           if(t=="timeout") {
             alert("[d001] Timeout while talking to the server.");
           } else {
             alert("[d002] Error while talking to the server: ".t);
           }
         },
         success: function(data){
           if( $(data).find('error').text() ){
             alert("Unable to get document details: "+$(data).find('error').text());
             return 1;
           }
           officialDocId = $(data).find('DocDetail').find('docid').text();
           $("#docid").text( officialDocId );
           $("#title").val( $(data).find('DocDetail').find('title').text() );
           $("#ocrtext").val( $(data).find('DocDetail').find('extractedText').text() );
           $("#docDate").val( $(data).find('DocDetail').find('docDate').text() );
           $("#scanDate").append(document.createTextNode( $(data).find('DocDetail').find('scanDate').text() ));
           $("#type").append(document.createTextNode( getTypeDescription($(data).find('DocDetail').find('type').text()) ));

           if( $(data).find('DocDetail').find('actionrequired').text() == "1" ) {
              $("#actionrequired").attr({checked: true}); 
           }
           else {
              $("#actionrequired").attr({checked: false});
           }

           if( $(data).find('DocDetail').find('hardcopyKept').text() == "1" ) {
              $("#hardcopyKept").attr({checked: true}); 
           }
           else {
              $("#hardcopyKept").attr({checked: false});
           }

           //
           // Doc type display options.
           if( $(data).find('DocDetail').find('type').text() == "1" ) { // ODF Documents
              $("#slider ul").append("<li><div class='scanImageContainer zoom'><img id='scanImage' " + 
                                     "alt='' src='/opendias/scans/"+officialDocId+"_thumb.png' />" + 
                                     "</div><button id='openImg'>Open ODF Document</button></li>");
              $("#scanImage").css("width", "300px");
              $("#openImg").bind('click', { docId: officialDocId }, 
                                  function(e){ 
                                      window.open("/opendias/scans/"+e.data.docId+".odt"); 
                                  });
           }


	   //JPG and scan data	
           else if( $(data).find('DocDetail').find('type').text() == "2" || $(data).find('DocDetail').find('type').text() == "4") {
             // Set images and default width
             for( x=1 ; x<=parseInt($(data).find('DocDetail').find('pages').text()) ; x++ ) {
               $("#slider ul").append("<li><div class='scanImageContainer zoom'><img id='scanImage"+x+"' " + 
                                      "alt='' src='/opendias/scans/"+officialDocId+"_"+x+".jpg' />" + 
                                      "</div><button id='openImg_"+x+"'>Open "+x+" Fullsize</button></li>");
               $("#scanImage"+x).css("width", "300px");
               $("#openImg_"+x).bind('click', { page: x, docId: officialDocId }, 
                                  function(e){ 
                                      window.open("/opendias/scans/"+e.data.docId+"_"+e.data.page+".jpg"); 
                                  });
             }

             // setup the slider
             $("#slider li").css("height", 45+($(data).find('DocDetail').find('y').text() * ( 300 / $(data).find('DocDetail').find('x').text() ))+"px" );
             if($(data).find('DocDetail').find('pages').text() != "1") {
               $("#slider").easySlider({prevText:'', nextText:''});
             }

             // make eachimage zoomable
             for( x=1 ; x<=parseInt($(data).find('DocDetail').find('pages').text()) ; x++ ) {
               $("#scanImage"+x).parent().gzoom({
                                   sW: 300,
                                   sH: $(data).find('DocDetail').find('y').text() * ( 300 / $(data).find('DocDetail').find('x').text() ),
                                   lW: $(data).find('DocDetail').find('x').text(),
                                   lH: $(data).find('DocDetail').find('y').text(), 
                                   lighbox: false
                                   });
             }

           }

           else if( $(data).find('DocDetail').find('type').text() == "3" ) { // PDF Documents
              $("#slider ul").append("<li><div id='pdf' class='scanImageContainer'><img id='scanImage' /></div>" + 
                                     "<button id='openImg'>Open PDF Document</button></li>");
              $("#scanImage").css("width", "300px");
              $("#scanImage").bind('error', { docId: officialDocId }, function (e) {
                                    $("#pdf").html("<div style='text-align: center; width: 100%'><p>PDF Thumbnail is not available.</p><p>Attempt to generate now.</p><button id='regenThumb'>Regenerate Thumbnail</button></div>");
                                    $('#regenThumb').bind('click', {docId: e.data.docId },
                                        function(d) {
                                            $.ajax({  url: "/opendias/dynamic",
                                                      dataType: "xml",
                                                      timeout: 10000,
                                                      data: { action: "regenerateThumb",
                                                              docid: d.data.docId},
                                                      cache: false,
                                                      type: "POST",
                                                      error: function( x, t, m ) {
                                                        if(t=="timeout") {
                                                          alert("[d001] Timeout while talking to the server.");
                                                        } else {
                                                          alert("[d002] Error while talking to the server: ".t);
                                                        }
                                                      },
                                                      success: function(data){
                                                        if( $(data).find('error').text() ){
                                                          alert("Unable to generate thumbnail: "+$(data).find('error').text());
                                                          return 1;
                                                        }
                                                        $("#pdf").html("<img id='scanImage' />");
                                                        $("#scanImage").css("width", "300px");
                                                        $("#scanImage").attr("src", "/opendias/scans/"+d.data.docId+"_thumb.jpg");
                                                      }
                                                    });
                                        });
                                    });
              $("#scanImage").attr("src", "/opendias/scans/"+officialDocId+"_thumb.jpg");
              $("#openImg").bind('click', { docId: officialDocId }, 
                                  function(e){ 
                                      window.open("/opendias/scans/"+e.data.docId+".pdf"); 
                                  });
           }

           $("#docDate").datepicker( {dateFormat:"yy/mm/dd"} );


           //
           // Load and setup Tags
           var tags = new Array();
           $(data).find('DocDetail').find('Tags').find('tag').each( function() {
                tags.push( $(this).text() );
           });
           $("#tags").val( tags.join() );
           $('#tags').tagsInput({
                autocomplete_url: '', // Were going to use the ui.autocomplete (since the plugins autocomplete stuff doesn't seam to work correctly. However, added this here as a hack to handle bluring correctly.
                onAddTag:function(tag) {
                          moveTag(tag,officialDocId,"addTag");
                          },
                onRemoveTag:function(tag) {
                          moveTag(tag,officialDocId,"removeTag");
                          },
           });
           $('#tags_tag').autocomplete({
               source: function( request, response ) {
                 $.ajax({
                   url: "/opendias/dynamic",
                   dataType: "json",
                   timeout: 10000,
                   type: "POST",
                   data: {
                     action: "tagsAutoComplete",
                     startsWith: request.term,
                     docid: officialDocId
                   },
                   error: function( x, t, m ) {
                     if(t=="timeout") {
                       alert("[d003] Timeout while talking to the server.");
                     } else {
                       alert("[d004] Error while talking to the server: ".t);
                     }
                   },
                   success: function( data ) {
                     response( $.map( data.results, function( item ) {
                       return {
                         label: item.tag,
                         value: item.tag
                       }
                     }));
                   }
                 });
               },
               minLength: 1, // because most tags are so short - and there are not that many tags,
               select: function( event, ui ) {
               //  log( ui.item ?  "Selected: " + ui.item.label : "Nothing selected, input was " + this.value);
               },
               open: function() {
                 $( this ).removeClass( "ui-corner-all" ).addClass( "ui-corner-top" );
               },
               close: function() {
                 $( this ).removeClass( "ui-corner-top" ).addClass( "ui-corner-all" );
               }
             });


           //
           // Load and setup document linkages
           $('#doclinks_tag').val($('#doclinks_tag').attr('data-default'));
           $('#doclinks_tag').bind('focus','',function(event) {
             if ($(this).val()==$(this).attr('data-default')) {
               $(this).val('');
             }
             $(this).css('color','#000000');
           });
           $('#doclinks_tag').bind('blur','',function(event) {
             if ($(this).val()=='') {
                $(this).val($(this).attr('data-default'));
                $(this).css('color','#676767');
              }
           });
           $(data).find('DocDetail').find('DocLinks').find('doc').each( function() {
             createDocLinkHtml( $(this).find('targetDocid').text(), $(this).find('targetTitle').text() );
           });
           $('#doclinks_tag').autocomplete({
               source: function( request, response ) {
                 $.ajax({
                   url: "/opendias/dynamic",
                   dataType: "json",
                   timeout: 10000,
                   type: "POST",
                   data: {
                     action: "titleAutoComplete",
                     startsWith: request.term,
                     notLinkedTo: officialDocId,
                   },
                   error: function( x, t, m ) {
                     if(t=="timeout") {
                       alert("[d005] Timeout while talking to the server.");
                     } else {
                       alert("[d006] Error while talking to the server: ".t);
                     }
                   },
                   success: function( data ) {
                     response( $.map( data.results, function( item ) {
                       return {
                         label: item.title,
                         value: item.docid
                       }
                     }));
                   }
                 });
               },
               minLength: 3, 
               select: function( event, ui ) {
                if (ui.item) {
                  createDocLinkHtml( ui.item.value, ui.item.label );
                  moveTag( ui.item.value, officialDocId, "addDoc");
                }
                return 0;
               },
               open: function() {
                 $( this ).removeClass( "ui-corner-all" ).addClass( "ui-corner-top" );
               },
               close: function() {
                 $( this ).removeClass( "ui-corner-top" ).addClass( "ui-corner-all" );
                 $(this).val($(this).attr('data-default'));
                 $(this).css('color','#676767');
               }
             });

         }
  });

  $('#title').autocomplete({
      source: function( request, response ) {
        $.ajax({
          url: "/opendias/dynamic",
          dataType: "json",
          timeout: 10000,
          type: "POST",
          data: {
            action: "titleAutoComplete",
            startsWith: request.term
          },
          error: function( x, t, m ) {
            if(t=="timeout") {
              alert("[d007] Timeout while talking to the server.");
            } else {
              alert("[d008] Error while talking to the server: ".t);
            }
          },
          success: function( data ) {
            response( $.map( data.results, function( item ) {
              return {
                label: item.title,
                value: item.title
              }
            }));
          }
        });
      },
      minLength: 2,
      select: function( event, ui ) {
        if(ui.item) {
          sendUpdate( 'title', ui.item.label);
        }
      },
      open: function() {
        $( this ).removeClass( "ui-corner-all" ).addClass( "ui-corner-top" );
      },
      close: function() {
        $( this ).removeClass( "ui-corner-top" ).addClass( "ui-corner-all" );
      }
    });

});

function getTypeDescription(iType) {

  if( iType == "1" ) {
    return "ODF Document";
  } else if( iType == "2" ) {
    return "Scanned Document";
  } else if( iType == "3" ) {
    return "PDF Document";
  } else if( iType == "4" ) {
    return "JPEG Image";
  } else {
    return "[unknown]";
  }

}

function createDocLinkHtml( val, lab ) {

  var ret = "<span>"
          + " <a href='/opendias/docDetail.html?docid=" + val + "'>"
          + "  " + lab
          + " </a>&nbsp;&nbsp;"
          + " </span>"
          + "<span id='deleteDocLink_"+val+"' class='clickable' title='Removing tag'>x</span>";

  $('<span>').addClass('tag').append( ret ).insertBefore( '#doclinks_addTag' );

  $('#deleteDocLink_'+val).bind( 'click', { v: val, i: officialDocId }, function(e) {
    moveTag( e.data.v, e.data.i, "removeDoc");
    $('#deleteDocLink_'+val).unbind('click');
    $(this).parent().remove();
  });

}
