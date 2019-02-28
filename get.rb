require 'open-uri'
require 'nokogiri'
require 'csv'

result = []
transcripts = open('https://steven-universe.fandom.com/wiki/Category:Transcripts').read
docTrans = Nokogiri::HTML transcripts
docTrans.css(".category-page__member-link").each do |node|
  puts node['href']
  subpage = open("https://steven-universe.fandom.com" + node['href']).read
  subpagedoc = Nokogiri::HTML subpage
  rows = subpagedoc.css(".bgrevo tr")
  rows.shift
  rows.pop
  rows.each do |row|
    if row.children.length == 2
      result << ["", row.children[1].content.strip.gsub(/\n/," ")]
    elsif row.children.length == 3
      result << [row.children[1].content.strip, row.children[2].content.strip.gsub(/\n/," ")]
    end
  end
end

CSV.open("dialogue.csv", "w") do |csv|
  result.each do |line|
    csv << line
  end
end
