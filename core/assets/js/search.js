
const search = {
    params: new URLSearchParams(location.search),
    decodeHtmlCharCodes: function(str) { 
        // https://stackoverflow.com/a/54346501
        return str.replace(/(&#(\d+);)/g, function(match, capture, charCode) {
            return String.fromCharCode(charCode);
        });
    },
    sanitizeQuery: function(queryString) {
        return queryString.replaceAll('\\', '');
    },
    HTMLEncode: function(str) {
        // https://stackoverflow.com/a/784765
        str = [...str];
        //    ^ es20XX spread to Array: keeps surrogate pairs
        let i = str.length, aRet = [];
      
        while (i--) {
            var iC = str[i].codePointAt(0);
            if (iC < 65 || iC > 127 || (iC>90 && iC<97)) {
                aRet[i] = '&#'+iC+';';
            } else {
                aRet[i] = str[i];
            }
        }
        return aRet.join('');
    },
    searchIndex: function(query) {
        if (typeof search.searchIndexArray == 'object') {
            let markup = '';
            search.searchIndexArray.forEach(function(item) {
                query = search.sanitizeQuery(query);
                const   itemContent = `${item.content} ${item.title}`,
                        content = search.decodeHtmlCharCodes(itemContent).toLowerCase(),
                        queryLowered = query.toLowerCase(),
                        queryLoweredEncoded = search.HTMLEncode(queryLowered),
                        occurrenceIndex = content.indexOf(queryLowered),
                        range = 200,
                        resultUrl = `${item.url}?text=${encodeURIComponent(query)}`;
                let lowIndex = occurrenceIndex - range,
                    highIndex = occurrenceIndex + query.length + range;
                if (lowIndex < 0) lowIndex = 0;
                if (highIndex > content.length - 1) highIndex = content.length - 1;
                let summary = content.slice(lowIndex, highIndex);
                summary = search.HTMLEncode(summary);
                summary = summary.replaceAll(queryLoweredEncoded, ` <mark class="fvs-wght-600 wb-break-all">${query}</mark> `);
                if (resultUrl.indexOf('/tags/?') === 0) item.title = libdocMessages.tagsList;
                if (content.indexOf(queryLowered) > -1) {
                    markup += search.renderSearchResult({url: resultUrl, title: item.title, summary: summary})
                }
            });
            const elTitle = document.querySelector('h1');
            elTitle.innerHTML += `<mark class="fvs-wght-600 wb-break-all">${query}</mark>`;
            if (markup == '') {
                searchResults.innerHTML = `<li>${libdocMessages.searchResultsNoResultForQuery} <mark class="fvs-wght-600 wb-break-all">${query}</mark></li>`;
            } else {
                searchResults.innerHTML = markup;
            }
            searchInput.value = query;
        }
    },
    renderSearchResult: function({url, title, summary}) {
        return `
            <li class="d-flex fd-column gap-2 | pb-5">
                <a  href="${url}"
                    class="fvs-wght-600 fs-5">
                    ${title}
                </a>
                <div class="wb-break-all fs-4" fs-3="xs">
                    ${summary}
                </div>
            </li>
        `;
    },
    update: function() {
        // console.log(query)
        fetch(libdocConfig.searchIndexUrl)
            .then(response => response.json())
            .then(searchIndexArray => {
                // console.log(json);
                search.searchIndexArray = searchIndexArray;
                const query = search.params.get('search');
                if (query !== null) {
                    if (query.length > 0) search.searchIndex(query);
                }
            })
            .catch(error => {
                // Handle the error
                console.log(error);
            });
    }
}
document.addEventListener('DOMContentLoaded', search.update);
